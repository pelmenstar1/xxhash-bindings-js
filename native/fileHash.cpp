#include <optional>
#include <stdexcept>

#include "exports.h"
#include "hashers.h"
#include "helpers.h"
#include "fileHashWorker.h"
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
    V8_PARSE_PROPERTY(options, offset, size_t, 0);
    V8_PARSE_PROPERTY(options, length, size_t, SIZE_MAX);

    NativeString nativePath = V8StringToNative(isolate, pathProp);
    FileHashingContext<Variant> hashingContext(nativePath.c_str(), offsetProp,
                                               lengthProp, seedProp);

    XxResult<Variant> result = HashFile(hashingContext, preferMapProp);

    info.GetReturnValue().Set(
        V8ValueConverter<XxResult<Variant>>::ConvertBack(isolate, result));
  } catch (const PlatformException& exc) {
    isolate->ThrowError(exc.WhatV8(isolate));
  } catch (const std::exception& exc) {
    isolate->ThrowError(Nan::New(exc.what()).ToLocalChecked());
  }
}

INSTANTIATE_HASH_FUNCTION(FileHash)