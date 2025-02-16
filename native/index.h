#include <napi.h>
#include "jsHashState.h"
#include "hashers.h"

struct CreateStateData {
  uint32_t variant;
  Napi::FunctionReference* constructor;

  CreateStateData(): variant(0), constructor(nullptr) {
  }

  CreateStateData(uint32_t variant, Napi::FunctionReference* constructor): variant(variant), constructor(constructor) {
  }
};

struct AddonData {
  CreateStateData variants[HASH_VARIANTS_COUNT];
};

class XxHashAddon : public Napi::Addon<XxHashAddon> {
  public:
    XxHashAddon(Napi::Env env, Napi::Object exports);

    Napi::Value OneshotHash(const Napi::CallbackInfo& info);
    Napi::Value CreateHashState(const Napi::CallbackInfo& info);
    Napi::Value FileHash(const Napi::CallbackInfo& info);
    Napi::Value FileHashAsync(const Napi::CallbackInfo& info);

  private:
    static uint32_t GetVariantData(const Napi::CallbackInfo& info) {
      return (uint32_t)reinterpret_cast<size_t>(info.Data());
    }
};