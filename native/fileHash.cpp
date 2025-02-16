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

#undef max

Napi::Value XxHashAddon::FileHash(const Napi::CallbackInfo& info) {
  uint32_t variant = GetVariantData(info);
  auto env = info.Env();

  if (info.Length() != 1) {
    throw Napi::Error::New(env, "Wrong number of arguments");
  }

  try {
    auto options = JsParseArgument<Napi::Object>(env, info[0], "options");

    auto path = JsParseProperty<Napi::String>(env, options, "path");
    uint64_t seed = JsParseSeedProperty(env, variant, options);
    auto preferMap = JsParseProperty<bool>(env, options, "preferMap", false);
    auto offset = JsParseProperty<uint64_t>(env, options, "offset", 0);
    auto length = JsParseProperty<uint64_t>(env, options, "length", std::numeric_limits<uint64_t>::max());

    auto nativePath = JsStringToCString<NativeChar>(path);
    HashWorkerContext hashContext(nativePath, offset, length);

    auto result = HashFile(hashContext, variant, seed, preferMap);

    return JsParseHashResult(env, variant, result);
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

Napi::Value XxHashAddon::FileHashAsync(const Napi::CallbackInfo& info) {
  class ReaderWorker : public Napi::AsyncWorker {
   public:
    ReaderWorker(uint32_t variant, uint64_t seed, NativeString path,
                 size_t fileOffset, size_t length, bool preferMap,
                 Napi::Function callback)
        : Napi::AsyncWorker(callback),
          _variant(variant),
          _seed(seed),
          _path(path),
          _fileOffset(fileOffset),
          _preferMap(preferMap),
          _length(length) {}

    void Execute() {
      HashWorkerContext hashContext(_path, _fileOffset, _length);

      try {
        _result = HashFile(hashContext, _variant, _seed, _preferMap);
      } catch (PlatformException& exc) {
        _error = exc.ErrorCode();
      }
    }

    void OnOK() {
      auto env = Env();

      if (_error == 0) {
        auto jsResult = JsParseHashResult(env, _variant, _result);

        Callback().Call({env.Undefined(), jsResult});
      } else {
        auto jsErrorMessage =
            PlatformException::FormatErrorToJsString(env, _error);
        auto jsError = Napi::Error::New(env, jsErrorMessage).Value();

        Callback().Call({jsError, env.Undefined()});
      }
    }

   private:
    uint32_t _variant;
    NativeString _path;
    size_t _fileOffset;
    size_t _length;
    uint64_t _seed;
    bool _preferMap;

    GenericHashResult _result;
    ErrorDesc _error = 0;
  };

  uint32_t variant = GetVariantData(info);
  Napi::Env env = info.Env();

  if (info.Length() != 2) {
    throw Napi::Error::New(env, "Wrong number of arguments");
  }

  Napi::Function callback;
  try {
    callback = JsParseArgument<Napi::Function>(env, info[1], "callback");
    auto options = JsParseArgument<Napi::Object>(env, info[0], "options");

    auto path = JsParseProperty<Napi::String>(env, options, "path");
    uint64_t seed = JsParseSeedProperty(env, variant, options);
    auto preferMap = JsParseProperty<bool>(env, options, "preferMap", false);
    auto offset = JsParseProperty<uint64_t>(env, options, "offset", 0);
    auto length = JsParseProperty<uint64_t>(env, options, "length", std::numeric_limits<uint64_t>::max());

    auto nativePath = JsStringToCString<NativeChar>(path);
    
    ReaderWorker* worker = new ReaderWorker(variant, seed, nativePath, offset,
                                            length, preferMap, callback);
    worker->Queue();
  } catch (const PlatformException& exc) {
    ExecuteCallbackWithErrorOrThrow(env, callback, exc.WhatJs(env));
  } catch (const std::exception& exc) {
    ExecuteCallbackWithErrorOrThrow(env, callback,
                                    Napi::String::New(env, exc.what()));
  }

  return env.Undefined();
}
