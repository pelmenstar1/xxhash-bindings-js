#pragma once

#include <napi.h>

#include <cstdint>

#include "hashers.h"

class JsHashStateObject : public Napi::ObjectWrap<JsHashStateObject> {
 public:
  JsHashStateObject(const Napi::CallbackInfo& info);

  Napi::Value Reset(const Napi::CallbackInfo& info);
  Napi::Value Update(const Napi::CallbackInfo& info);
  Napi::Value GetResult(const Napi::CallbackInfo& info);

  static Napi::Function Init(Napi::Env env);

 private:
  XxHashDynamicState _state;
  uint32_t _variant;
  uint64_t _seed;
};
