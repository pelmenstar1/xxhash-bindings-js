#include "exports.h"
#include "hashers.h"
#include "helpers.h"
#include "v8HashAdapter.h"

template <int Variant>
inline void XxHashBase(const Nan::FunctionCallbackInfo<v8::Value>& info) {
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
    auto optSeed = V8HashAdapter<Variant>::GetSeed(isolate, info[1]);
    if (!optSeed.has_value()) {
      THROW_INVALID_ARG_TYPE(1, "number, bigint, undefined or null");
    }

    seed = optSeed.value();
  }

  auto result = XxHasher<Variant>::Process(isolate, buffer.data, buffer.length, seed);

  info.GetReturnValue().Set(
      V8HashAdapter<Variant>::TransformResult(isolate, result));
}

#define ONESHOT_SPEC(name, variant)                             \
  void name(const Nan::FunctionCallbackInfo<v8::Value>& info) { \
    XxHashBase<variant>(info);                                  \
  }

ONESHOT_SPEC(XxHash32, H32)
ONESHOT_SPEC(XxHash64, H64)
ONESHOT_SPEC(XxHash3, H3)
ONESHOT_SPEC(XxHash3_128, H3_128)
