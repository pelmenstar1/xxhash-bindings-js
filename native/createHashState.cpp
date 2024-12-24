#include "exports.h"
#include "hashers.h"
#include "helpers.h"
#include "v8HashState.h"

template <int Variant>
void CreateHashState(const Nan::FunctionCallbackInfo<v8::Value>& info) {
  auto isolate = info.GetIsolate();

  v8::Local<v8::Value> seedArg =
      info.Length() > 0 ? info[0]
                        : v8::Undefined(isolate).As<v8::Value>();

  v8::MaybeLocal<v8::Object> result = V8HashStateObject<Variant>::NewInstance(
      isolate->GetCurrentContext(), seedArg);

  v8::Local<v8::Object> resultValue;
  if (result.ToLocal(&resultValue)) {
    info.GetReturnValue().Set(resultValue);
  }
}

INSTANTIATE_HASH_FUNCTION(CreateHashState)