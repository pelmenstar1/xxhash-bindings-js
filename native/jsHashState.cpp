#include "jsHashState.h"

#include <napi.h>

#include "index.h"
#include "jsObjectParser.h"
#include "jsUtils.h"

Napi::Function JsHashStateObject::Init(Napi::Env env) {
  return DefineClass(
      env, "XxHashState",
      {InstanceMethod("reset", &JsHashStateObject::Reset, napi_default_method),
       InstanceMethod("update", &JsHashStateObject::Update,
                      napi_default_method),
       InstanceMethod("result", &JsHashStateObject::GetResult,
                      napi_default_method)});
}

JsHashStateObject::JsHashStateObject(const Napi::CallbackInfo& info)
    : Napi::ObjectWrap<JsHashStateObject>(info) {
  auto env = info.Env();

  if (info.Length() != 2) {
    throw Napi::Error::New(env, "Wrong number of arguments");
  }

  uint32_t variant = JsParseArgument<uint32_t>(env, info[0], "variant");
  uint64_t seed = JsParseSeedArgument(env, variant, info[1]);

  _seed = seed;
  _variant = variant;
  _state = XxHashDynamicState(variant, seed);
}

Napi::Value JsHashStateObject::Reset(const Napi::CallbackInfo& info) {
  _state.Reset(_seed);

  return info.Env().Undefined();
}

Napi::Value JsHashStateObject::Update(const Napi::CallbackInfo& info) {
  auto env = info.Env();

  if (info.Length() != 1) {
    throw Napi::Error::New(env, "Wrong number of arguments");
  }

  auto data = JsParseArgument<RawSizedArray>(env, info[0], "data");

  _state.Update(data.data, data.length);

  return env.Undefined();
}

Napi::Value JsHashStateObject::GetResult(const Napi::CallbackInfo& info) {
  auto env = info.Env();
  XXH128_hash_t result = _state.GetResult();

  return JsParseHashResult(env, _variant, result);
}
