#include <nan.h>

#include <memory>
#include <stdexcept>

#include "directoryHashUtils.h"
#include "exports.h"
#include "fileHashWorker.h"
#include "hashers.h"
#include "helpers.h"
#include "platform/directoryIterator.h"
#include "platform/nativeString.h"
#include "platform/platformError.h"
#include "v8ObjectParser.h"

template <int Variant>
HashWorker<Variant>* SelectHashWorker(XxSeed<Variant> seed, bool preferMap) {
  if (preferMap) {
    return new MapHashWorker<Variant>(seed);
  }

  return new BlockHashWorker<Variant>(seed);
}

template <int Variant>
struct BaseHashHandler {
 public:
  v8::Isolate* isolate;
  v8::Local<v8::String> path;
  XxSeed<Variant> seed;
  bool preferMap;
  v8::Local<v8::Function> acceptFile;

  BaseHashHandler(v8::Isolate* isolate, v8::Local<v8::Object> options)
      : isolate(isolate) {
    path = V8ParseProperty<v8::Local<v8::String>>(isolate, options, "path");
    seed = V8ParseProperty<XxSeed<Variant>>(isolate, options, "seed", 0);
    preferMap = V8ParseProperty(isolate, options, "preferMap", false);
    acceptFile = V8ParseProperty(isolate, options, "acceptFile",
                                 v8::Local<v8::Function>());
  }
};

template <int Variant>
struct HashHandler : public BaseHashHandler<Variant> {
  v8::Local<v8::Function> onFile;

  HashHandler(v8::Isolate* isolate, v8::Local<v8::Object> options)
      : BaseHashHandler<Variant>(isolate, options) {
    onFile =
        V8ParseProperty<v8::Local<v8::Function>>(isolate, options, "onFile");
  }

  bool OnFile(v8::Local<v8::String> key, v8::Local<v8::Value> value) {
    auto isolate = BaseHashHandler<Variant>::isolate;

    const int argc = 2;
    v8::Local<v8::Value> argv[argc];
    argv[0] = key;
    argv[1] = value;

    auto maybeResult = onFile->Call(isolate->GetCurrentContext(),
                                    v8::Undefined(isolate), argc, argv);

    return !maybeResult.IsEmpty();
  }

  void OnReturn(const Nan::FunctionCallbackInfo<v8::Value>& info) {}
};

template <int Variant>
struct ToMapHashHandler : public BaseHashHandler<Variant> {
  v8::Local<v8::Map> resultMap;

  ToMapHashHandler(v8::Isolate* isolate, v8::Local<v8::Object> options)
      : BaseHashHandler<Variant>(isolate, options) {
    resultMap = v8::Map::New(isolate);
  }

  bool OnFile(v8::Local<v8::String> key, v8::Local<v8::Value> value) {
    auto isolate = BaseHashHandler<Variant>::isolate;

    resultMap->Set(isolate->GetCurrentContext(), key, value);
    return true;
  }

  void OnReturn(const Nan::FunctionCallbackInfo<v8::Value>& info) {
    info.GetReturnValue().Set(resultMap);
  }
};

template <int Variant, typename Handler>
void DirectoryHashBase(const Nan::FunctionCallbackInfo<v8::Value>& info) {
  auto isolate = info.GetIsolate();

  try {
    v8::Local<v8::Object> options;
    switch (info.Length()) {
      case 1:
        V8_PARSE_ARGUMENT(options, 0, v8::Local<v8::Object>);
        break;
      default:
        throw std::runtime_error("Wrong number of arguments");
    }

    Handler handler(isolate, options);

    auto nativePath = V8StringToCString<NativeChar>(isolate, handler.path);
    DirectoryIterator<> dirIter(nativePath);

    std::unique_ptr<HashWorker<Variant>> worker(
        SelectHashWorker<Variant>(handler.seed, handler.preferMap));

    DirectoryEntry<> entry;
    while (dirIter.Next(&entry)) {
      auto v8FileName = CStringToV8String(isolate, entry.fileName);

      switch (CallAcceptFile(isolate, handler.acceptFile, v8FileName)) {
        case FileGateResult::True: {
          HashWorkerContext<> context(entry.fullPath, 0, SIZE_MAX);
          auto result = worker->Process(context);

          auto v8Result =
              V8ValueConverter<XxResult<Variant>>::ConvertBack(isolate, result);

          if (!handler.OnFile(v8FileName, v8Result)) {
            return;
          }

          break;
        }
        case FileGateResult::False:
          break;
        case FileGateResult::Error:
          // On error, we should return from the C++ function immediately.
          return;
      }
    }

    handler.OnReturn(info);
  } catch (const PlatformException& exc) {
    isolate->ThrowError(exc.WhatV8(isolate));
  } catch (const std::exception& exc) {
    isolate->ThrowError(Nan::New(exc.what()).ToLocalChecked());
  }
}

template <int Variant>
void DirectoryHash(const Nan::FunctionCallbackInfo<v8::Value>& info) {
  DirectoryHashBase<Variant, HashHandler<Variant>>(info);
}

template <int Variant>
void DirectoryToMapHash(const Nan::FunctionCallbackInfo<v8::Value>& info) {
  DirectoryHashBase<Variant, ToMapHashHandler<Variant>>(info);
}

INSTANTIATE_HASH_FUNCTION(DirectoryHash)
INSTANTIATE_HASH_FUNCTION(DirectoryToMapHash)
