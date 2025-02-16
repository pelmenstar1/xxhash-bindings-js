#include <node_api.h>

#include "hashers.h"
#include "index.h"
#include "jsHashState.h"

Napi::Value XxHashAddon::CreateHashState(const Napi::CallbackInfo& info) {
  auto env = info.Env();
  auto data = (CreateStateData*)info.Data();

  return data->constructor->New({Napi::Number::New(env, data->variant), info[0]});
}
