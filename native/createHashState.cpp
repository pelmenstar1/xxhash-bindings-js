#include "exports.h"
#include "hashers.h"
#include "helpers.h"
#include "v8HashState.h"

template <int Variant>
void CreateHashState(const Nan::FunctionCallbackInfo<v8::Value>& info) {
  v8::Local<v8::Value> seedArg =
      info.Length() > 0 ? info[0]
                        : v8::Undefined(info.GetIsolate()).As<v8::Value>();

  v8::MaybeLocal<v8::Object> result = V8HashStateObject<Variant>::NewInstance(
      info.GetIsolate()->GetCurrentContext(), seedArg);

  if (!result.IsEmpty()) {
    info.GetReturnValue().Set(result.ToLocalChecked());
  }
}

INSTANTIATE_HASH_FUNCTION(CreateHashState)