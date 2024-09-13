#include "oneshots.h"

#include "hashers.h"
#include "helpers.h"

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
    THROW_INVALID_ARG_TYPE(1, Uint8Array);
  }

  auto buffer = bufferArg.As<v8::Uint8Array>()->Buffer();
  uint8_t* data = (uint8_t*)buffer->Data();
  size_t length = buffer->ByteLength();

  V8OptionalSeed optSeed = {};

  if (argCount == 2) {
    auto seedArg = info[1];

    CHECK_SEED_UNDEFINED(seedArg);

    optSeed = seedArg;
  }

  v8::Local<v8::Value> result =
      XxHasher<Variant>::Process(isolate, data, length, seedArg);

  info.GetReturnValue().Set(result);
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