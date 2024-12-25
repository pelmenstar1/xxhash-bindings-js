#include <optional>
#include <stdexcept>

#include "exports.h"
#include "hashers.h"
#include "helpers.h"
#include "platform/blockReader.h"
#include "platform/memoryMap.h"
#include "platform/nativeString.h"
#include "platform/platformError.h"
#include "v8HashAdapter.h"
#include "v8ObjectParser.h"

template <int Variant>
struct FileHashingContext {
  NativeString path;
  bool preferMap;
  size_t offset;
  size_t length;
  XxSeed<Variant> seed;

  FileHashingContext(NativeString path, bool preferMap, size_t offset,
                     size_t length, XxSeed<Variant> seed)
      : path(path),
        preferMap(preferMap),
        offset(offset),
        length(length),
        seed(seed) {}
};

template <int Variant>
XxResult<Variant> BlockHashFile(const FileHashingContext<Variant>& context) {
  XxHashState<Variant> state = {};
  state.Init(context.seed);

  auto reader = BlockReader::Open(context.path, context.offset, context.length);

  while (true) {
    auto block = reader.ReadBlock();

    if (block.length == 0) {
      break;
    }

    state.Update(block.data, block.length);
  }

  return state.GetResult();
}

template <int Variant>
XxResult<Variant> MapHashFile(const FileHashingContext<Variant>& context) {
  MemoryMappedFile file;
  bool isCompatible = file.Open(context.path, context.offset, context.length);

  if (!isCompatible) {
    return BlockHashFile(context);
  }

  size_t size = file.GetSize();
  XxResult<Variant> result;

  file.Access(
      [&](const uint8_t* address) {
        result =
            XxHasher<Variant>::Process(address, size, context.seed);
      },
      [&] {
        throw std::runtime_error("IO error occurred while reading the file");
      });

  return result;
}

template <int Variant>
XxResult<Variant> HashFile(const FileHashingContext<Variant>& context) {
  return context.preferMap ? MapHashFile<Variant>(context)
                           : BlockHashFile<Variant>(context);
}

template <int Variant>
FileHashingContext<Variant> GetHashingContextFromV8Options(
    v8::Local<v8::Context> context, v8::Local<v8::Value> options) {
  auto isolate = context->GetIsolate();

  if (!options->IsObject()) {
    throw std::runtime_error("Argument 'options' should be an object");
  }

  auto optionsObj = options.As<v8::Object>();

  V8_PARSE_PROPERTY(optionsObj, path, "string", v8::Local<v8::String>);

  V8_PARSE_PROPERTY_OPTIONAL(optionsObj, seed, "number, bigint or undefined",
                             XxSeed<Variant>, 0);

  V8_PARSE_PROPERTY_OPTIONAL(optionsObj, preferMap, "boolean or undefined",
                             bool, false);

  V8_PARSE_PROPERTY_OPTIONAL(optionsObj, offset, "number, bigint or undefined",
                             size_t, 0);

  V8_PARSE_PROPERTY_OPTIONAL(optionsObj, length, "number, bigint or undefined",
                             size_t, SIZE_MAX);

  NativeString nativePath = V8StringToNative(isolate, pathProp);

  return {nativePath, preferMapProp, offsetProp, lengthProp, seedProp};
}

template <int Variant>
void FileHash(const Nan::FunctionCallbackInfo<v8::Value>& info) {
  v8::Isolate* isolate = info.GetIsolate();
  try {
    v8::Local<v8::Context> context = isolate->GetCurrentContext();
    int argCount = info.Length();

    if (argCount < 1) {
      throw std::runtime_error("Wrong number of arguments");
    }

    auto hashingContext =
        GetHashingContextFromV8Options<Variant>(context, info[0]);

    XxResult<Variant> result = HashFile(hashingContext);

    info.GetReturnValue().Set(
        V8HashAdapter<Variant>::TransformResult(isolate, result));
  } catch (const PlatformException& exc) {
    isolate->ThrowError(exc.WhatV8(isolate));
  } catch (const std::exception& exc) {
    isolate->ThrowError(Nan::New(exc.what()).ToLocalChecked());
  }
}

INSTANTIATE_HASH_FUNCTION(FileHash)