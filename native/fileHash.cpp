#include <optional>
#include <stdexcept>

#include "exports.h"
#include "hashers.h"
#include "helpers.h"
#include "platform/blockReader.h"
#include "platform/memoryMap.h"
#include "platform/nativeString.h"
#include "platform/platformError.h"
#include "v8ObjectParser.h"
#include "v8Utils.h"

template <int Variant>
struct FileHashingContext {
  NativeString path;
  size_t offset;
  size_t length;
  XxSeed<Variant> seed;

  FileHashingContext(NativeString path, size_t offset, size_t length,
                     XxSeed<Variant> seed)
      : path(path), offset(offset), length(length), seed(seed) {}
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
        result = XxHasher<Variant>::Process(address, size, context.seed);
      },
      [&] {
        throw std::runtime_error("IO error occurred while reading the file");
      });

  return result;
}

template <int Variant>
XxResult<Variant> HashFile(const FileHashingContext<Variant>& context,
                           bool preferMap) {
  return preferMap ? MapHashFile<Variant>(context)
                   : BlockHashFile<Variant>(context);
}

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
    FileHashingContext<Variant> hashingContext(nativePath, offsetProp,
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