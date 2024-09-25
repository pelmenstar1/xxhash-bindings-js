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
  auto seedArg = info[1];

  if (!bufferArg->IsUint8Array()) {
    THROW_INVALID_ARG_TYPE(1, "Uint8Array");
  }

  auto buffer = bufferArg.As<v8::Uint8Array>()->Buffer();
  uint8_t* data = (uint8_t*)buffer->Data();
  size_t length = buffer->ByteLength();

  XxSeed<Variant> seed = 0;

  if (argCount == 2) {
    auto optSeed = V8HashAdapter<Variant>::GetSeed(isolate, info[1]);
    if (!optSeed.has_value()) {
      THROW_INVALID_ARG_TYPE(1, "number, bigint, undefined or null");
    }

    seed = optSeed.value();
  }

  auto result = XxHasher<Variant>::Process(isolate, data, length, seed);

  info.GetReturnValue().Set(
      V8HashAdapter<Variant>::TransformResult(isolate, result));
}

void XxHash32(const Nan::FunctionCallbackInfo<v8::Value>& info) {
  XxHashBase<H32>(info);
}

void XxHash64(const Nan::FunctionCallbackInfo<v8::Value>& info) {
  XxHashBase<H64>(info);
}

void XxHash3(const Nan::FunctionCallbackInfo<v8::Value>& info) {
  XxHashBase<H3>(info);
}

void XxHash3_128(const Nan::FunctionCallbackInfo<v8::Value>& info) {
  XxHashBase<H3_128>(info);
}