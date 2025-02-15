#include <node_api.h>

#include "hashers.h"
#include "index.h"
#include "jsHashState.h"

template <int Variant>
Napi::Value XxHashAddon::CreateHashState(const Napi::CallbackInfo& info) {
  auto env = info.Env();
  auto cons = reinterpret_cast<Napi::FunctionReference*>(info.Data());

  return cons->New({Napi::Number::New(env, Variant), info[0]});;
}

INSTANTIATE_ADDON_METHOD(CreateHashState)