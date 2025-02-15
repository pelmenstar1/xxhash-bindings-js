#include "hashers.h"
#include "index.h"
#include "jsObjectParser.h"

template <int Variant>
Napi::Value XxHashAddon::OneshotHash(const Napi::CallbackInfo& info) {
  auto env = info.Env();

  RawSizedArray data;
  XxSeed<Variant> seed = 0;

  switch (info.Length()) {
    case 2:
      JS_PARSE_ARGUMENT(seed, 1, XxSeed<Variant>, 0);
    case 1:
      JS_PARSE_ARGUMENT(data, 0, RawSizedArray);
      break;
    default:
      throw Napi::Error::New(env, "Wrong number of arguments");
  }

  auto result = XxHashTraits<Variant>::Oneshot(data.data, data.length, seed);

  return JsValueConverter<XxResult<Variant>>::ConvertBack(env, result);
}

INSTANTIATE_ADDON_METHOD(OneshotHash)