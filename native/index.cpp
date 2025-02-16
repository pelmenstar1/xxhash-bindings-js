#include "index.h"

#include "jsHashState.h"

#define FUNCTION_SET_ITEM(name, function, data) \
  InstanceMethod(name, &XxHashAddon::function, napi_default_method, data)

#define FUNCTION_SET(suffix, function)                              \
  FUNCTION_SET_ITEM("xxhash32_" #suffix, function, (void*)H32),     \
      FUNCTION_SET_ITEM("xxhash64_" #suffix, function, (void*)H64), \
      FUNCTION_SET_ITEM("xxhash3_" #suffix, function, (void*)H3),  \
      FUNCTION_SET_ITEM("xxhash3_128_" #suffix, function, (void*)H3_128)

XxHashAddon::XxHashAddon(Napi::Env env, Napi::Object exports) {
  Napi::FunctionReference* stateCons = new Napi::FunctionReference(
      Napi::Persistent(JsHashStateObject::Init(env)));

  AddonData* data = new AddonData();

  for (uint32_t i = 0; i < HASH_VARIANTS_COUNT; i++) {
    data->variants[i] = CreateStateData(i, stateCons);
  }

  env.AddCleanupHook([stateCons, data]() {
    stateCons->Reset();

    delete stateCons;
    delete data;
  });

  DefineAddon(exports,
              {
                  FUNCTION_SET(oneshot, OneshotHash),
                  FUNCTION_SET(file, FileHash),
                  FUNCTION_SET(fileAsync, FileHashAsync),

                  FUNCTION_SET_ITEM("xxhash32_createState", CreateHashState,
                                    &data->variants[H32]),
                  FUNCTION_SET_ITEM("xxhash64_createState", CreateHashState,
                                    &data->variants[H64]),
                  FUNCTION_SET_ITEM("xxhash3_createState", CreateHashState,
                                    &data->variants[H3]),
                  FUNCTION_SET_ITEM("xxhash3_128_createState", CreateHashState,
                                    &data->variants[H3_128]),

              });
}

NODE_API_ADDON(XxHashAddon)
