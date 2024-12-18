#include <iostream>
#include <optional>

#include "errorMacro.h"
#include "exports.h"
#include "hashers.h"
#include "helpers.h"
#include "platform/blockReader.h"
#include "platform/memoryMap.h"
#include "v8HashAdapter.h"

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

    if (readResult.IsSuccess()) {
      auto block = readResult.GetValue();
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
HashResult<Variant> MapProcessFile(const FileHashingContext<Variant>& context) {
  auto isolate = context.isolate;

  MemoryMappedFile file;
  auto openResult = file.Open(isolate, context.ToOpenOptions());

  if (openResult.IsIncompatible()) {
    return BlockProcessFile(context);
  }

  if (openResult.IsError()) {
    openResult.ThrowException(isolate);
    return {};
  }

  size_t size = file.GetSize();
  HashResult<Variant> result = {};

  file.Access(
      [&](const uint8_t* address) {
        result =
            XxHasher<Variant>::Process(isolate, address, size, context.seed);
      },
      [&] { isolate->ThrowError("IO error occurred while reading the file"); });

  return result;
}

template <int Variant>
static HashResult<Variant> DynamicProcessFile(
    const FileHashingContext<Variant>& context, bool preferMap) {
  return preferMap ? MapProcessFile<Variant>(context)
                   : BlockProcessFile<Variant>(context);
}

template <int Variant>
void FileHash(const Nan::FunctionCallbackInfo<v8::Value>& info) {
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

  bool preferMap = true;
  if (argCount >= 3) {
    auto preferMapArg = info[2];
    if (preferMapArg->IsUndefined()) {
      preferMap = false;
    } else if (preferMapArg->IsBoolean()) {
      preferMap = preferMapArg->BooleanValue(isolate);
    } else {
      THROW_INVALID_ARG_TYPE(2, "boolean or undefined");
    }
  }

  size_t offset = 0;
  size_t length = SIZE_MAX;

  if (argCount >= 4) {
    auto optOffset = V8GetUInt64Optional(isolate, info[3]);

    if (!optOffset.has_value()) {
      THROW_INVALID_ARG_TYPE(3, "number, bigint, undefined or null");
    }

    offset = (size_t)optOffset.value();
  }

  if (argCount >= 5) {
    auto optLength = V8GetUInt64Optional(isolate, info[4], SIZE_MAX);

    if (!optLength.has_value()) {
      THROW_INVALID_ARG_TYPE(4, "number, bigint, undefined or null");
    }

    length = (size_t)optLength.value();
  }

  auto result = DynamicProcessFile<Variant>(
      FileHashingContext<Variant>(isolate, pathValue, offset, length, seed),
      preferMap);
  if (!result.has_value()) {
    return;
  }

  info.GetReturnValue().Set(
      V8HashAdapter<Variant>::TransformResult(isolate, result.value()));
}

INSTANTIATE_HASH_FUNCTION(FileHash)