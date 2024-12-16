#include "exports.h"
#include "hashers.h"
#include "v8HashState.h"

template <HashVariant Variant>
void CreateXxHashState(const Nan::FunctionCallbackInfo<v8::Value>& info) {
  v8::Local<v8::Value> seedArg = info.Length() > 0 ? info[0] : Nan::Undefined();

  auto result = V8HashStateObject<Variant>::NewInstance(seedArg);
  info.GetReturnValue().Set(result);
}

#define STATE_SPEC(name, variant)                               \
  void name(const Nan::FunctionCallbackInfo<v8::Value>& info) { \
    CreateXxHashState<variant>(info);                           \
  }

STATE_SPEC(CreateXxHash32State, H32)
STATE_SPEC(CreateXxHash64State, H64)
STATE_SPEC(CreateXxHash3State, H3)
STATE_SPEC(CreateXxHash3_128_State, H3_128)