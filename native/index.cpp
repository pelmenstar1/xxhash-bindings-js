#include "index.h"

#include "jsHashState.h"

#define FUNCTION_SET(suffix, function, ...)                                  \
  InstanceMethod("xxhash32_" #suffix, &XxHashAddon::function<H32>,           \
                 napi_default_method __VA_ARGS__),                           \
      InstanceMethod("xxhash64_" #suffix, &XxHashAddon::function<H64>,       \
                     napi_default_method __VA_ARGS__),                       \
      InstanceMethod("xxhash3_" #suffix, &XxHashAddon::function<H3>,         \
                     napi_default_method __VA_ARGS__),                       \
      InstanceMethod("xxhash3_128_" #suffix, &XxHashAddon::function<H3_128>, \
                     napi_default_method __VA_ARGS__)

XxHashAddon::XxHashAddon(Napi::Env env, Napi::Object exports) {
  Napi::FunctionReference* stateCons = new Napi::FunctionReference(
      Napi::Persistent(JsHashStateObject::Init(env)));

  env.AddCleanupHook([stateCons]() {
    stateCons->Reset();
    delete stateCons;
  });

  DefineAddon(exports,
              {
                  FUNCTION_SET(oneshot, OneshotHash),
                  FUNCTION_SET(createState, CreateHashState, , stateCons),
                  FUNCTION_SET(file, FileHash),
                  FUNCTION_SET(fileAsync, FileHashAsync),
              });
}

NODE_API_ADDON(XxHashAddon)
