#include "fileHashing.h"

#include <iostream>
#include <optional>

#include "errorMacro.h"
#include "hashers.h"
#include "helpers.h"
#include "platform/blockReader.h"
#include "platform/memoryMap.h"

#define CHECK_PATH(pathArg)            \
  if (!(pathArg)->IsString()) {        \
    THROW_INVALID_ARG_TYPE(1, string); \
  }

enum FileHashingType { MAP = 0, BLOCK = 1 };

typedef std::optional<v8::Local<v8::Value>> V8HashResult;

template <int Variant, int Type>
struct FileHasher {};

template <int Variant>
struct FileHasher<Variant, MAP> {
  static V8HashResult ProcessFile(v8::Isolate* isolate,
                                  v8::Local<v8::String> pathValue,
                                  V8OptionalSeed seedValue);
};

template <int Variant>
struct FileHasher<Variant, BLOCK> {
  static V8HashResult ProcessFile(v8::Isolate* isolate,
                                  v8::Local<v8::String> pathValue,
                                  V8OptionalSeed seedValue);
};

template <int Variant>
static V8HashResult DynamicProcessFile(v8::Isolate* isolate,
                                       v8::Local<v8::String> pathValue,
                                       V8OptionalSeed seedValue,
                                       FileHashingType type) {
  switch (type) {
    case BLOCK:
      return FileHasher<Variant, BLOCK>::ProcessFile(isolate, pathValue,
                                                     seedValue);
    case MAP:
      return FileHasher<Variant, MAP>::ProcessFile(isolate, pathValue,
                                                   seedValue);
    default:
      FATAL_ERROR("Invalid processing type");
      return {};
  }
}

template <int Variant>
V8HashResult FileHasher<Variant, MAP>::ProcessFile(
    v8::Isolate* isolate, v8::Local<v8::String> pathValue,
    V8OptionalSeed seedValue) {
  MemoryMappedFile file;
  auto openResult = file.Open(isolate, pathValue);

  if (openResult.IsError()) {
    openResult.ThrowException(isolate);
    return {};
  }

  const uint8_t* address = file.GetAddress();
  uint64_t size = file.GetSize();

  return XxHasher<Variant>::Process(isolate, address, (size_t)size, seedValue);
}

template <int Variant>
V8HashResult FileHasher<Variant, BLOCK>::ProcessFile(
    v8::Isolate* isolate, v8::Local<v8::String> pathValue,
    V8OptionalSeed seedValue) {
  XxHashState<Variant> state = XxHasher<Variant>::CreateState(isolate);
  BlockReader reader = {};

  auto openResult = reader.Open(isolate, pathValue);
  if (openResult.IsError()) {
    openResult.ThrowException(isolate);

    return {};
  }

  bool initResult = state.Init(seedValue);
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
    return MAP;
  }

  isolate->ThrowError("Invalid file hashing type");
  return {};
}

template <int Variant, bool RequireSeed>
void XxHashBaseFile(const Nan::FunctionCallbackInfo<v8::Value>& info) {
  v8::Isolate* isolate = info.GetIsolate();
  v8::Local<v8::Context> context = isolate->GetCurrentContext();
  int argCount = info.Length();

  if (argCount < 1 || argCount > 3) {
    THROW_INVALID_ARG_COUNT;
  }

  auto pathArg = info[0];

  CHECK_PATH(pathArg);

  auto pathValue = pathArg->ToString(context).ToLocalChecked();

  V8OptionalSeed optSeed = {};
  if (argCount >= 2) {
    auto seedArg = info[1];
    CHECK_SEED_UNDEFINED(seedArg);

    optSeed = seedArg;
  } else if (RequireSeed) {
    THROW_INVALID_ARG_COUNT;
  }

  FileHashingType type = MAP;
  if (argCount == 3) {
    auto typeArg = info[2];

    auto optType = GetFileHashingType(isolate, typeArg);

    if (!optType.has_value()) {
      return;
    }

    type = optType.value();
  }

  V8HashResult result =
      DynamicProcessFile<Variant>(isolate, pathValue, optSeed, type);
  if (!result.has_value()) {
    return;
  }

  info.GetReturnValue().Set(result.value());
}

void XxHash32File(const Nan::FunctionCallbackInfo<v8::Value>& info) {
  XxHashBaseFile<H32, true>(info);
}

void XxHash64File(const Nan::FunctionCallbackInfo<v8::Value>& info) {
  XxHashBaseFile<H64, true>(info);
}

void XxHash3File(const Nan::FunctionCallbackInfo<v8::Value>& info) {
  XxHashBaseFile<H3, false>(info);
}

void XxHash3_128_File(const Nan::FunctionCallbackInfo<v8::Value>& info) {
  XxHashBaseFile<H3_128, false>(info);
}
