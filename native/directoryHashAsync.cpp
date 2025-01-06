#include <nan.h>

#include <atomic>
#include <mutex>
#include <vector>

#include "directoryHashUtils.h"
#include "exports.h"
#include "hashers.h"
#include "helpers.h"
#include "platform/blockReader.h"
#include "platform/directoryIterator.h"
#include "platform/platformError.h"
#include "v8ObjectParser.h"

// Responsible for parsing 'options' from the JS side, coordinating actions when
// a file is processed, when all files are read, when error happens.
template <int Variant>
struct BaseAsyncHashHandler {
  v8::Isolate* isolate;
  v8::Local<v8::String> path;
  XxSeed<Variant> seed;
  v8::Local<v8::Function> acceptFile;
  v8::Global<v8::Function> callback;

  BaseAsyncHashHandler(v8::Isolate* isolate, v8::Local<v8::Object> options,
                       v8::Local<v8::Function> callback)
      : isolate(isolate) {
    path = V8ParseProperty<v8::Local<v8::String>>(isolate, options, "path");
    seed = V8ParseProperty<XxSeed<Variant>>(isolate, options, "seed", 0);

    acceptFile = V8ParseProperty(isolate, options, "acceptFile",
                                 v8::Local<v8::Function>());
    this->callback = v8::Global<v8::Function>(isolate, callback);
  }

  // Called when a file is processed. Should be thread-safe.
  virtual bool OnFile(const std::string& key, XxResult<Variant> value) = 0;

  // Guaranteed to be called only once.
  virtual void OnEnd() = 0;

  void OnError(const char* message) {
    v8::HandleScope scope(isolate);

    v8::Local<v8::String> v8Message =
        v8::String::NewFromUtf8(isolate, message).ToLocalChecked();

    OnError(v8Message);
  }

  void OnError(ErrorDesc error) {
    v8::HandleScope scope(isolate);

    v8::Local<v8::String> v8Message =
        PlatformException::FormatErrorToV8String(isolate, error);

    OnError(v8Message);
  }

  void OnError(v8::Local<v8::String> message) {
    OnException(v8::Exception::Error(message));
  }

  void OnException(v8::Local<v8::Value> exc) { ExecuteCallback(exc); }

  template <typename... Args>
  inline v8::MaybeLocal<v8::Value> ExecuteCallback(Args... args) {
    Nan::AsyncResource resource("DirectoryHash");

    const int argc = sizeof...(Args);
    v8::Local<v8::Value> argv[argc] = {args...};

    return resource.runInAsyncScope(v8::Object::New(isolate),
                                    callback.Get(isolate), argc, argv);
  }
};

template <int Variant>
struct AsyncHashHandler : public BaseAsyncHashHandler<Variant> {
  v8::Global<v8::Function> onFile;

  AsyncHashHandler(v8::Isolate* isolate, v8::Local<v8::Object> options,
                   v8::Local<v8::Function> callback)
      : BaseAsyncHashHandler<Variant>(isolate, options, callback) {
    onFile = v8::Global<v8::Function>(
        isolate,
        V8ParseProperty<v8::Local<v8::Function>>(isolate, options, "onFile"));
  }

  bool OnFile(const std::string& key, XxResult<Variant> value) override {
    auto isolate = BaseAsyncHashHandler<Variant>::isolate;

    v8::Locker locker(isolate);
    v8::Isolate::Scope isolateScope(isolate);
    v8::HandleScope handleScope(isolate);
    v8::TryCatch tryCatch(isolate);

    auto v8Key = CStringToV8String<char>(isolate, key);
    auto v8Value =
        V8ValueConverter<XxResult<Variant>>::ConvertBack(isolate, value);

    Nan::AsyncResource resource("DirectoryHash");

    const int argc = 2;
    v8::Local<v8::Value> argv[argc];
    argv[0] = v8Key;
    argv[1] = v8Value;

    resource.runInAsyncScope(v8::Object::New(isolate), onFile.Get(isolate),
                             argc, argv);

    if (tryCatch.HasCaught()) {
      BaseAsyncHashHandler<Variant>::OnException(tryCatch.Exception());
      return false;
    }

    return true;
  }

  void OnEnd() override {
    auto isolate = BaseAsyncHashHandler<Variant>::isolate;
    v8::HandleScope scope(isolate);

    BaseAsyncHashHandler<Variant>::ExecuteCallback(v8::Undefined(isolate));
  }
};

template <int Variant>
struct AsyncToMapHashHandler : public BaseAsyncHashHandler<Variant> {
  struct MapEntry {
    std::string key;
    XxResult<Variant> value;

    MapEntry(std::string key, XxResult<Variant> value)
        : key(key), value(value) {}
  };

  std::vector<MapEntry> _entries;
  std::mutex _entriesMutex;

  AsyncToMapHashHandler(v8::Isolate* isolate, v8::Local<v8::Object> options,
                        v8::Local<v8::Function> callback)
      : BaseAsyncHashHandler<Variant>(isolate, options, callback) {}

  bool OnFile(const std::string& key, XxResult<Variant> value) override {
    _entriesMutex.lock();
    _entries.push_back(MapEntry(key, value));
    _entriesMutex.unlock();

    return true;
  }

  void OnEnd() override {
    v8::Isolate* isolate = BaseAsyncHashHandler<Variant>::isolate;
    v8::HandleScope scope(isolate);

    v8::Local<v8::Context> context = isolate->GetCurrentContext();

    v8::Local<v8::Map> resultMap = v8::Map::New(isolate);
    for (const MapEntry& entry : _entries) {
      v8::Local<v8::String> v8Key = CStringToV8String<char>(isolate, entry.key);
      auto v8Value = V8ValueConverter<XxResult<Variant>>::ConvertBack(
          isolate, entry.value);

      resultMap->Set(context, v8Key, v8Value);
    }

    BaseAsyncHashHandler<Variant>::ExecuteCallback(v8::Undefined(isolate),
                                                   resultMap);
  }
};

template <int Variant>
using HandlerPtr = std::shared_ptr<BaseAsyncHashHandler<Variant>>;

struct AsyncHashStats {
  std::atomic<size_t> fileCompleted;
  std::atomic<size_t> totalFiles;

  AsyncHashStats() : fileCompleted({0}), totalFiles({SIZE_MAX}) {}
};

template <int Variant>
class AsyncBlockReaderImpl : public AsyncBlockReader {
 public:
  AsyncBlockReaderImpl(std::string&& key, HandlerPtr<Variant> handler,
                       std::shared_ptr<AsyncHashStats> stats)
      : AsyncBlockReader(0, SIZE_MAX),
        _hashState(handler->seed),
        _stats(stats),
        _key(std::move(key)),
        _handler(handler) {}

  void OnBlock(const uint8_t* data, size_t length) override {
    _hashState.Update(data, length);
  }

  void OnEnd() override {
    auto result = _hashState.GetResult();
    _handler->OnFile(_key, result);

    auto stats = _stats.get();
    size_t fileCompleted = stats->fileCompleted.fetch_add(1) + 1;
    size_t totalFiles = stats->totalFiles.load(std::memory_order_acquire);

    if (fileCompleted == totalFiles) {
      _handler->OnEnd();
    }
  }

  void OnError(const char* message) override { _handler->OnError(message); }

 private:
  XxHashState<Variant> _hashState;
  std::string _key;
  HandlerPtr<Variant> _handler;
  std::shared_ptr<AsyncHashStats> _stats;
};

template <int Variant, typename Handler>
void DirectoryHashAsyncBase(const Nan::FunctionCallbackInfo<v8::Value>& info) {
  auto isolate = info.GetIsolate();
  if (info.Length() != 2) {
    isolate->ThrowError("Wrong number of arguments");
    return;
  }

  v8::Local<v8::Function> callback;
  try {
    callback =
        V8ParseArgument<v8::Local<v8::Function>>(isolate, info[1], "callback");

    auto options =
        V8ParseArgument<v8::Local<v8::Object>>(isolate, info[0], "options");

    HandlerPtr<Variant> handler(new Handler(isolate, options, callback));
    std::shared_ptr<AsyncHashStats> stats(new AsyncHashStats());

    v8::Local<v8::Function> acceptFile = handler->acceptFile;
    auto nativePath = V8StringToCString<NativeChar>(isolate, handler->path);

    DirectoryIterator<char> dirIter(nativePath);
    DirectoryEntry<char> dirEntry;

    auto uvLoop = Nan::GetCurrentEventLoop();
    size_t entryCount = 0;

    while (dirIter.Next(&dirEntry)) {
      const char* fullPath = dirEntry.fullPath;
      const char* fileName = dirEntry.fileName;

      if (acceptFile.IsEmpty()) {
        goto scheduleFile;
      }

      {
        auto key = CStringToV8String(isolate, fileName);
        auto maybeResult = V8CallFunction(isolate, acceptFile, key);
        auto gateResult = ConvertToFileGateResult(isolate, maybeResult);

        if (gateResult == FileGateResult::False) {
          continue;
        } else if (gateResult == FileGateResult::Error) {
          return;
        }
      }

    scheduleFile:
      auto fileReader = new AsyncBlockReaderImpl<Variant>(std::string(fileName),
                                                          handler, stats);
      fileReader->Schedule(uvLoop, fullPath);

      entryCount++;
    }

    if (entryCount == 0) {
      handler->OnEnd();
    } else {
      stats.get()->totalFiles.store(entryCount, std::memory_order_release);

      info.GetReturnValue().SetUndefined();
    }
  } catch (const PlatformException& exc) {
    ExecuteCallbackWithErrorOrThrow(isolate, callback, exc.WhatV8(isolate));
  } catch (const std::exception& exc) {
    auto v8What = v8::String::NewFromUtf8(isolate, exc.what()).ToLocalChecked();

    ExecuteCallbackWithErrorOrThrow(isolate, callback, v8What);
  }
}

template <int Variant>
void DirectoryHashAsync(const Nan::FunctionCallbackInfo<v8::Value>& info) {
  DirectoryHashAsyncBase<Variant, AsyncHashHandler<Variant>>(info);
}

template <int Variant>
void DirectoryToMapHashAsync(const Nan::FunctionCallbackInfo<v8::Value>& info) {
  DirectoryHashAsyncBase<Variant, AsyncToMapHashHandler<Variant>>(info);
}

INSTANTIATE_HASH_FUNCTION(DirectoryHashAsync)
INSTANTIATE_HASH_FUNCTION(DirectoryToMapHashAsync)
