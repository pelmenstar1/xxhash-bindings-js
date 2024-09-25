#include <iostream>
#include <optional>

#include "errorMacro.h"
#include "exports.h"
#include "hashers.h"
#include "helpers.h"
#include "platform/blockReader.h"
#include "platform/memoryMap.h"
#include "v8HashAdapter.h"

enum FileHashingType { MAP = 0, BLOCK = 1 };

const FileHashingType DEFAULT_HASHING_TYPE = MAP;

template <int Variant>
using HashResult = std::optional<XxResult<Variant>>;

template <int Variant>
struct FileHashingContext {
  v8::Isolate* isolate;
  v8::Local<v8::String> path;
  size_t offset;
  size_t length;
  XxSeed<Variant> seed;

  FileHashingContext(v8::Isolate* isolate, v8::Local<v8::String> path,
                     size_t offset, size_t length, XxSeed<Variant> seed)
      : isolate(isolate),
        path(path),
        offset(offset),
        length(length),
        seed(seed) {}

  FileOpenOptions ToOpenOptions() const { return {path, offset, length}; }
};

template <int Variant>
HashResult<Variant> MapProcessFile(const FileHashingContext<Variant>& context) {
  auto isolate = context.isolate;
  MemoryMappedFile file;
  auto openResult = file.Open(isolate, context.ToOpenOptions());

  if (openResult.IsError()) {
    openResult.ThrowException(isolate);
    return {};
  }

  const uint8_t* address = file.GetAddress();
  size_t size = file.GetSize();

  return XxHasher<Variant>::Process(isolate, address, size, context.seed);
}

template <int Variant>
HashResult<Variant> BlockProcessFile(
    const FileHashingContext<Variant>& context) {
  auto isolate = context.isolate;
  XxHashState<Variant> state = {};
  BlockReader reader = {};

  auto openResult = reader.Open(isolate, context.ToOpenOptions());
  if (openResult.IsError()) {
    openResult.ThrowException(isolate);

    return {};
  }

  bool initResult = state.Init(context.seed);
  if (!initResult) {
    isolate->ThrowError("Out of memory");

    return {};
  }

  while (true) {
    auto readResult = reader.ReadBlock();

    if (readResult.IsSucess()) {
      auto block = readResult.GetBlock();
      if (block.length == 0) {
        break;
      }

      state.Update(block.data, block.length);
    } else {
      readResult.ThrowException(isolate);

      return {};
    }
  }

  return state.GetResult();
}

template <int Variant>
static HashResult<Variant> DynamicProcessFile(
    FileHashingType type, const FileHashingContext<Variant>& context) {
  switch (type) {
    case BLOCK:
      return BlockProcessFile<Variant>(context);
    case MAP:
      return MapProcessFile<Variant>(context);
    default:
      FATAL_ERROR("Invalid processing type");
      return {};
  }
}

std::optional<FileHashingType> GetFileHashingType(
    v8::Isolate* isolate, v8::Local<v8::Value> typeValue) {
  if (typeValue->IsNumber()) {
    auto intValueMaybe = typeValue->Int32Value(isolate->GetCurrentContext());

    if (intValueMaybe.IsJust()) {
      auto value = intValueMaybe.FromJust();

      switch (value) {
        case MAP:
        case BLOCK:
          return (FileHashingType)value;
      }
    }
  } else if (typeValue->IsNullOrUndefined()) {
    return DEFAULT_HASHING_TYPE;
  }

  return {};
}

template <int Variant>
void XxHashBaseFile(const Nan::FunctionCallbackInfo<v8::Value>& info) {
  v8::Isolate* isolate = info.GetIsolate();
  v8::Local<v8::Context> context = isolate->GetCurrentContext();
  int argCount = info.Length();

  if (argCount < 1) {
    THROW_INVALID_ARG_COUNT;
  }

  auto pathArg = info[0];

  if (!pathArg->IsString()) {
    THROW_INVALID_ARG_TYPE(1, "string");
  }

  auto pathValue = pathArg->ToString(context).ToLocalChecked();

  XxSeed<Variant> seed = 0;
  if (argCount >= 2) {
    auto optSeed = V8HashAdapter<Variant>::GetSeed(isolate, info[1]);
    if (!optSeed.has_value()) {
      THROW_INVALID_ARG_TYPE(1, "number, bigint, undefined or null");
    }

    seed = optSeed.value();
  }

  FileHashingType type = DEFAULT_HASHING_TYPE;
  if (argCount >= 3) {
    auto optType = GetFileHashingType(isolate, info[2]);

    if (!optType.has_value()) {
      Nan::ThrowTypeError("Invalid mapping type");
      return;
    }

    type = optType.value();
  }

  size_t offset = 0;
  size_t length = SIZE_MAX;

  if (argCount >= 4) {
    auto optOffset = V8GetUInt64Optional(isolate, info[3]);

    if (!optOffset.has_value()) {
      THROW_INVALID_ARG_TYPE(1, "number, bigint, undefined or null");
    }

    offset = (size_t)optOffset.value();
  }

  if (argCount >= 5) {
    auto optLength = V8GetUInt64Optional(isolate, info[4]);

    if (!optLength.has_value()) {
      THROW_INVALID_ARG_TYPE(1, "number, bigint, undefined or null");
    }

    length = (size_t)optLength.value();
  }

  auto result = DynamicProcessFile<Variant>(
      type,
      FileHashingContext<Variant>(isolate, pathValue, offset, length, seed));
  if (!result.has_value()) {
    return;
  }

  info.GetReturnValue().Set(
      V8HashAdapter<Variant>::TransformResult(isolate, result.value()));
}

#define FILE_SPEC(name, variant)                                \
  void name(const Nan::FunctionCallbackInfo<v8::Value>& info) { \
    XxHashBaseFile<variant>(info);                              \
  }

FILE_SPEC(XxHash32File, H32)
FILE_SPEC(XxHash64File, H64)
FILE_SPEC(XxHash3File, H3)
FILE_SPEC(XxHash3_128_File, H3_128)