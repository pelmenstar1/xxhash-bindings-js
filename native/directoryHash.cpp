#include <nan.h>

#include <memory>
#include <stdexcept>

#include "exports.h"
#include "fileHashWorker.h"
#include "hashers.h"
#include "helpers.h"
#include "platform/directoryIterator.h"
#include "platform/nativeString.h"
#include "platform/platformError.h"
#include "v8ObjectParser.h"

enum FileGateResult { True = 1, False = 0, Error = -1 };

class FileGate {
 private:
  v8::Local<v8::Function> _func;

 public:
  FileGate(v8::Local<v8::Function> func) : _func(func) {}

  FileGateResult Accept(v8::Isolate* isolate, v8::Local<v8::String> fileName) {
    if (_func.IsEmpty()) {
      return FileGateResult::True;
    }

    const int argc = 1;
    v8::Local<v8::Value> argv[argc];
    argv[0] = fileName;

    v8::MaybeLocal<v8::Value> maybeResult = _func->Call(
        isolate->GetCurrentContext(), v8::Undefined(isolate), argc, argv);

    v8::Local<v8::Value> result;
    if (maybeResult.ToLocal(&result)) {
      // Use more general way to convert the returned value to boolean
      // in order to be more aligned with the "minimum" implementation which
      // just does if (acceptGate ...) 
      return (FileGateResult)result->ToBoolean(isolate)->Value();
    }

    return FileGateResult::Error;
  }
};

template <int Variant>
HashWorker<Variant>* SelectHashWorker(XxSeed<Variant> seed, bool preferMap) {
  if (preferMap) {
    return new MapHashWorker<Variant>(seed);
  }

  return new BlockHashWorker<Variant>(seed);
}

template <int Variant>
void DirectoryHash(const Nan::FunctionCallbackInfo<v8::Value>& info) {
  auto isolate = info.GetIsolate();
  auto context = isolate->GetCurrentContext();

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
    V8_PARSE_PROPERTY(options, acceptFile, v8::Local<v8::Function>,
                      v8::Local<v8::Function>());

    FileGate fileGate(acceptFileProp);

    auto resultMap = v8::Map::New(isolate);

    NativeString nativePath = V8StringToNative(isolate, pathProp);

    DirectoryIterator dirIter(nativePath);

    std::unique_ptr<HashWorker<Variant>> worker(
        SelectHashWorker<Variant>(seedProp, preferMapProp));

    DirectoryEntry entry;
    while (dirIter.Next(&entry)) {
      auto key = V8ConvertNativeString(isolate, entry.fileName);

      switch (fileGate.Accept(isolate, key)) {
        case FileGateResult::True: {
          auto result = worker->Process(entry.fullPath, 0, SIZE_MAX);

          auto v8Result =
              V8ValueConverter<XxResult<Variant>>::ConvertBack(isolate, result);

          resultMap->Set(context, key, v8Result);
          break;
        }
        case FileGateResult::False:
          break;
        case FileGateResult::Error:
          // On error, we should return from the C++ function immediately.
          return;
      }
    }

    info.GetReturnValue().Set(resultMap);
  } catch (const PlatformException& exc) {
    isolate->ThrowError(exc.WhatV8(isolate));
  } catch (const std::exception& exc) {
    isolate->ThrowError(Nan::New(exc.what()).ToLocalChecked());
  }
}

INSTANTIATE_HASH_FUNCTION(DirectoryHash)
