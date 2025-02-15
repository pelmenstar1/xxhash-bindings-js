#include <napi.h>
#include "jsHashState.h"

#define DECLARE_INSTANCE_METHOD(name) \
  template <int Variant>            \
  Napi::Value name(const Napi::CallbackInfo& info)

#define _INSTANTIATE_ADDON_METHOD(name, variant) \
  template Napi::Value XxHashAddon::name<variant>(const Napi::CallbackInfo& info);

#define INSTANTIATE_ADDON_METHOD(name) \
  _INSTANTIATE_ADDON_METHOD(name, H32) \
  _INSTANTIATE_ADDON_METHOD(name, H64) \
  _INSTANTIATE_ADDON_METHOD(name, H3)  \
  _INSTANTIATE_ADDON_METHOD(name, H3_128)

class XxHashAddon : public Napi::Addon<XxHashAddon> {
  public:
    XxHashAddon(Napi::Env env, Napi::Object exports);

    DECLARE_INSTANCE_METHOD(OneshotHash);
    DECLARE_INSTANCE_METHOD(CreateHashState);
    DECLARE_INSTANCE_METHOD(FileHash);
    DECLARE_INSTANCE_METHOD(FileHashAsync);
};