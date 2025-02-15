#include <napi.h>

#include <limits>
#include <stdexcept>

#include "fileHashWorker.h"
#include "hashers.h"
#include "index.h"
#include "jsObjectParser.h"
#include "jsUtils.h"
#include "platform/nativeString.h"
#include "platform/platformError.h"

template <int Variant>
Napi::Value XxHashAddon::FileHash(const Napi::CallbackInfo& info) {
  auto env = info.Env();

  if (info.Length() != 1) {
    throw Napi::Error::New(env, "Wrong number of arguments");
  }

  try {
    auto options = JsParseArgument<Napi::Object>(env, info[0], "options");

    JS_PARSE_PROPERTY(options, path, Napi::String);
    JS_PARSE_PROPERTY(options, seed, XxSeed<Variant>, 0);
    JS_PARSE_PROPERTY(options, preferMap, bool, false);
    JS_PARSE_PROPERTY(options, offset, uint64_t, 0);
    JS_PARSE_PROPERTY(options, length, uint64_t,
                      std::numeric_limits<uint64_t>::max());

    auto nativePath = JsStringToCString<NativeChar>(pathProp);
    HashWorkerContext hashContext(nativePath, offsetProp, lengthProp);

    auto result = HashFile<Variant>(hashContext, seedProp, preferMapProp);

    return JsValueConverter<XxResult<Variant>>::ConvertBack(env, result);
  } catch (const PlatformException& exc) {
    Napi::Error::New(env, exc.WhatJs(env)).ThrowAsJavaScriptException();

    return env.Undefined();
  }
}

void ExecuteCallbackWithErrorOrThrow(Napi::Env env,
                                     const Napi::Function& callback,
                                     const Napi::String& message) {
  auto error = Napi::Error::New(env, message);

  if (callback.IsUndefined()) {
    error.ThrowAsJavaScriptException();
  } else {
    callback.Call({error.Value()});
  }
}

template <int Variant>
Napi::Value XxHashAddon::FileHashAsync(const Napi::CallbackInfo& info) {
  class ReaderWorker : public Napi::AsyncWorker {
   public:
    ReaderWorker(Napi::Function callback, XxSeed<Variant> seed,
                 NativeString path, size_t fileOffset, size_t length,
                 bool preferMap)
        : Napi::AsyncWorker(callback),
          _seed(seed),
          _path(path),
          _fileOffset(fileOffset),
          _length(length),
          _result(0),
          _error(0) {}

    void Execute() {
      HashWorkerContext hashContext(_path, _fileOffset, _length);

      try {
        _result = HashFile<Variant>(hashContext, _seed, _preferMap);
      } catch (PlatformException& exc) {
        _error = exc.ErrorCode();
      }
    }

    void OnOK() {
      auto env = Env();

      if (_error == 0) {
        auto jsResult =
            JsValueConverter<XxResult<Variant>>::ConvertBack(env, _result);

        Callback().Call({env.Undefined(), jsResult});
      } else {
        auto jsErrorMessage = PlatformException::FormatErrorToJsString(env, _error);
        auto jsError = Napi::Error::New(env, jsErrorMessage).Value();

        Callback().Call({jsError, env.Undefined()});
      }
    }

   private:
    NativeString _path;
    size_t _fileOffset;
    size_t _length;
    XxSeed<Variant> _seed;
    bool _preferMap;

    XxResult<Variant> _result;
    ErrorDesc _error;
  };

  Napi::Env env = info.Env();

  if (info.Length() != 2) {
    throw Napi::Error::New(env, "Wrong number of arguments");
  }

  Napi::Function callback;
  try {
    callback = JsParseArgument<Napi::Function>(env, info[1], "callback");
    auto options = JsParseArgument<Napi::Object>(env, info[0], "options");

    JS_PARSE_PROPERTY(options, path, Napi::String);
    JS_PARSE_PROPERTY(options, seed, XxSeed<Variant>, 0);
    JS_PARSE_PROPERTY(options, preferMap, bool, false);
    JS_PARSE_PROPERTY(options, offset, uint64_t, 0);
    JS_PARSE_PROPERTY(options, length, uint64_t,
                      std::numeric_limits<uint64_t>::max());
    
    auto nativePath = JsStringToCString<NativeChar>(pathProp);
    
    ReaderWorker* worker = new ReaderWorker(
        callback, seedProp, nativePath, offsetProp,
        lengthProp, preferMapProp);
    worker->Queue();
  } catch (const PlatformException& exc) {
    ExecuteCallbackWithErrorOrThrow(env, callback, exc.WhatJs(env));
  } catch (const std::exception& exc) {
    ExecuteCallbackWithErrorOrThrow(env, callback,
                                    Napi::String::New(env, exc.what()));
  }

  return env.Undefined();
}

INSTANTIATE_ADDON_METHOD(FileHash)
INSTANTIATE_ADDON_METHOD(FileHashAsync)