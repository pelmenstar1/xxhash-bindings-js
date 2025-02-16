#include "hashers.h"
#include "index.h"
#include "jsObjectParser.h"
#include "jsUtils.h"

Napi::Value XxHashAddon::OneshotHash(const Napi::CallbackInfo& info) {
  auto env = info.Env();
  uint32_t variant = GetVariantData(info);

  RawSizedArray data;
  uint64_t seed = 0;

  switch (info.Length()) {
    case 2:
      seed = JsParseSeedArgument(env, variant, info[1]);
    case 1:
      data = JsParseArgument<RawSizedArray>(env, info[0], "data");
      break;
    default:
      throw Napi::Error::New(env, "Wrong number of arguments");
  }

  auto result = XxHashDynamicState::Oneshot(variant, data.data, data.length, seed);

  return JsParseHashResult(env, variant, result);
}
