#include <optional>

#include "errorMacro.h"
#include "exports.h"
#include "hashers.h"
#include "helpers.h"
#include "platform/blockReader.h"
#include "platform/memoryMap.h"
#include "v8HashAdapter.h"
#include "v8ObjectParser.h"

template <int Variant>
using HashResult = std::optional<XxResult<Variant>>;

template <int Variant>
struct FileHashingContext {
  v8::Isolate* isolate;
  v8::Local<v8::String> path;
  bool preferMap;
  size_t offset;
  size_t length;
  XxSeed<Variant> seed;

  FileHashingContext(v8::Isolate* isolate, v8::Local<v8::String> path,
                     bool preferMap, size_t offset, size_t length,
                     XxSeed<Variant> seed)
      : isolate(isolate),
        path(path),
        preferMap(preferMap),
        offset(offset),
        length(length),
        seed(seed) {}

  FileOpenOptions ToOpenOptions() const { return {path, offset, length}; }
};

template <int Variant>
HashResult<Variant> BlockHashFile(const FileHashingContext<Variant>& context) {
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
HashResult<Variant> MapHashFile(const FileHashingContext<Variant>& context) {
  auto isolate = context.isolate;

  MemoryMappedFile file;
  auto openResult = file.Open(isolate, context.ToOpenOptions());

  if (openResult.IsIncompatible()) {
    return BlockHashFile(context);
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
HashResult<Variant> HashFile(const FileHashingContext<Variant>& context) {
  return context.preferMap ? MapHashFile<Variant>(context)
                           : BlockHashFile<Variant>(context);
}

template <int Variant>
std::optional<FileHashingContext<Variant>> GetHashingContextFromV8Options(
    v8::Local<v8::Context> context, v8::Local<v8::Value> options) {
  auto isolate = context->GetIsolate();

  if (!options->IsObject()) {
    isolate->ThrowError("Argument 'options' should be an object");
    return {};
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

  auto hashingContext = FileHashingContext<Variant>(
      isolate, pathProp, preferMapProp, offsetProp, lengthProp, seedProp);

  return {hashingContext};
}

template <int Variant>
void FileHash(const Nan::FunctionCallbackInfo<v8::Value>& info) {
  v8::Isolate* isolate = info.GetIsolate();
  v8::Local<v8::Context> context = isolate->GetCurrentContext();
  int argCount = info.Length();

  if (argCount < 1) {
    THROW_INVALID_ARG_COUNT;
  }

  auto optionsArg = info[0];
  auto hashingContext =
      GetHashingContextFromV8Options<Variant>(context, info[0]);
  if (!hashingContext.has_value()) {
    return;
  }

  auto result = HashFile(hashingContext.value());
  if (!result.has_value()) {
    return;
  }

  info.GetReturnValue().Set(
      V8HashAdapter<Variant>::TransformResult(isolate, result.value()));
}

INSTANTIATE_HASH_FUNCTION(FileHash)