#include <nan.h>

#include <stdexcept>
#include <limits>

#include "exports.h"
#include "fileHashWorker.h"
#include "hashers.h"
#include "helpers.h"
#include "platform/nativeString.h"
#include "platform/platformError.h"
#include "v8ObjectParser.h"
#include "v8Utils.h"

template <int Variant>
void FileHash(const Nan::FunctionCallbackInfo<v8::Value>& info) {
  v8::Isolate* isolate = info.GetIsolate();

  try {
    v8::Local<v8::Object> options;
    switch (info.Length()) {
      case 1:
        V8_PARSE_ARGUMENT(options, 0, v8::Local<v8::Object>);
        break;
      default:
        throw std::runtime_error("Wrong number of arguments");
    }

    V8_PARSE_PROPERTY(options, path, v8::Local<v8::String>);
    V8_PARSE_PROPERTY(options, seed, XxSeed<Variant>, 0);
    V8_PARSE_PROPERTY(options, preferMap, bool, false);
    V8_PARSE_PROPERTY(options, offset, uint64_t, 0);
    V8_PARSE_PROPERTY(options, length, uint64_t, std::numeric_limits<uint64_t>::max());

    auto nativePath = V8StringToCString<NativeChar>(isolate, pathProp);
    HashWorkerContext<> hashContext(nativePath.c_str(), offsetProp, lengthProp);

    auto result = HashFile<Variant>(hashContext, seedProp, preferMapProp);

    info.GetReturnValue().Set(
        V8ValueConverter<XxResult<Variant>>::ConvertBack(isolate, result));
  } catch (const PlatformException& exc) {
    isolate->ThrowError(exc.WhatV8(isolate));
  } catch (const std::exception& exc) {
    isolate->ThrowError(Nan::New(exc.what()).ToLocalChecked());
  }
}

template <int Variant>
void FileHashAsync(const Nan::FunctionCallbackInfo<v8::Value>& info) {
  class AsyncBlockReaderImpl : public AsyncBlockReader {
   public:
    AsyncBlockReaderImpl(v8::Isolate* isolate,
                         v8::Global<v8::Function>& callback,
                         XxSeed<Variant> seed, size_t fileOffset, size_t length)
        : AsyncBlockReader(fileOffset, length),
          _isolate(isolate),
          _hashState(seed),
          _callback(std::move(callback)) {}

    void OnBlock(const uint8_t* data, size_t length) override {
      _hashState.Update(data, length);
    }

    void OnEnd() override {
      v8::Locker locker(_isolate);
      v8::Isolate::Scope isolateScope(_isolate);
      v8::HandleScope handleScope(_isolate);
      
      auto result = _hashState.GetResult();

      auto v8Result =
          V8ValueConverter<XxResult<Variant>>::ConvertBack(_isolate, result);

      ExecuteCallback(v8::Undefined(_isolate), v8Result);
    }

    void OnError(const char* message) override {
      v8::Locker locker(_isolate);
      v8::Isolate::Scope isolateScope(_isolate);
      v8::HandleScope handleScope(_isolate);

      auto v8Message =
          v8::String::NewFromUtf8(_isolate, message).ToLocalChecked();
      auto v8Error = v8::Exception::Error(v8Message);

      ExecuteCallback(v8Error, v8::Undefined(_isolate));
    }

   private:
    v8::Isolate* _isolate;
    XxHashState<Variant> _hashState;
    v8::Global<v8::Function> _callback;

    void ExecuteCallback(v8::Local<v8::Value> error,
                         v8::Local<v8::Value> result) {
      Nan::AsyncResource resource("FileHash");

      const int argc = 2;
      v8::Local<v8::Value> argv[argc];
      argv[0] = error;
      argv[1] = result;

      resource.runInAsyncScope(v8::Object::New(_isolate),
                               _callback.Get(_isolate), argc, argv);
    }
  };

  v8::Isolate* isolate = info.GetIsolate();

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

    V8_PARSE_PROPERTY(options, path, v8::Local<v8::String>);
    V8_PARSE_PROPERTY(options, seed, XxSeed<Variant>, 0);
    V8_PARSE_PROPERTY(options, preferMap, bool, false);
    V8_PARSE_PROPERTY(options, offset, size_t, 0);
    V8_PARSE_PROPERTY(options, length, size_t, SIZE_MAX);

    auto nativePath = V8StringToCString<char>(isolate, pathProp);

    v8::Global<v8::Function> globalCallback(isolate, callback);

    AsyncBlockReaderImpl* impl = new AsyncBlockReaderImpl(
        isolate, globalCallback, seedProp, offsetProp, lengthProp);
    impl->Schedule(Nan::GetCurrentEventLoop(), nativePath.c_str());
  } catch (const PlatformException& exc) {
    ExecuteCallbackWithErrorOrThrow(isolate, callback, exc.WhatV8(isolate));
  } catch (const std::exception& exc) {
    auto v8What = v8::String::NewFromUtf8(isolate, exc.what()).ToLocalChecked();

    ExecuteCallbackWithErrorOrThrow(isolate, callback, v8What);
  }
}

INSTANTIATE_HASH_FUNCTION(FileHash)
INSTANTIATE_HASH_FUNCTION(FileHashAsync)