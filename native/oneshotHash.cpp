#include "exports.h"
#include "hashers.h"
#include "helpers.h"
#include "v8HashAdapter.h"
#include "v8ObjectParser.h"

template <int Variant>
void OneshotHash(const Nan::FunctionCallbackInfo<v8::Value>& info) {
  v8::Isolate* isolate = info.GetIsolate();

  int argCount = info.Length();
  if (argCount < 1 || argCount > 2) {
    THROW_INVALID_ARG_COUNT;
  }

  auto bufferArg = info[0];
  if (!bufferArg->IsUint8Array()) {
    THROW_INVALID_ARG_TYPE(1, "Uint8Array");
  }

  auto buffer = V8GetBackingStorage(bufferArg.As<v8::Uint8Array>());

  XxSeed<Variant> seed = 0;

  if (argCount == 2) {
    auto optSeed = V8ValueParser<XxSeed<Variant>>()(isolate, info[1], 0);
    if (!optSeed.has_value()) {
      THROW_INVALID_ARG_TYPE(1, "number, bigint, undefined or null");
    }

    seed = optSeed.value();
  }

  auto result =
      XxHasher<Variant>::Process(isolate, buffer.data, buffer.length, seed);

  info.GetReturnValue().Set(
      V8HashAdapter<Variant>::TransformResult(isolate, result));
}

INSTANTIATE_HASH_FUNCTION(OneshotHash)