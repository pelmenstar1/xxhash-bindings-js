#include "jsHashState.h"

#include <napi.h>

#include "index.h"
#include "jsObjectParser.h"

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
  uint64_t seed = 0;

  if (variant == H32) {
    seed = JsParseArgument<uint32_t>(env, info[1], "seed", 0);
  } else {
    seed = JsParseArgument<uint64_t>(env, info[1], "seed", 0);
  }

  _seed = seed;
  _variant = variant;

  switch (variant) {
    case H32:
      _state = new XxHashState32(seed);
      break;
    case H64:
      _state = new XxHashState64(seed);
      break;
    case H3:
      _state = new XxHashState3(seed);
      break;
    case H3_128:
      _state = new XxHashState3_128(seed);
      break;
    default:
      throw Napi::Error::New(env, "Invalid variant");
  }
}

Napi::Value JsHashStateObject::Reset(const Napi::CallbackInfo& info) {
  _state->Reset(_seed);

  return info.Env().Undefined();
}

Napi::Value JsHashStateObject::Update(const Napi::CallbackInfo& info) {
  auto env = info.Env();

  RawSizedArray data;

  switch (info.Length()) {
    case 1:
      JS_PARSE_ARGUMENT(data, 0, RawSizedArray);
      break;
    default:
      throw Napi::Error::New(env, "Wrong number of arguments");
  }

  _state->Update(data.data, data.length);

  return env.Undefined();
}

Napi::Value JsHashStateObject::GetResult(const Napi::CallbackInfo& info) {
  auto env = info.Env();

  XXH128_hash_t result = _state->GetResult();

  switch (_variant) {
    case H32:
      return Napi::Number::New(env, result.low64);
    case H64:
    case H3:
      return Napi::BigInt::New(env, result.low64);
    case H3_128:
      return JsValueConverter<XXH128_hash_t>::ConvertBack(env, result);
  }

  return env.Undefined();
}
