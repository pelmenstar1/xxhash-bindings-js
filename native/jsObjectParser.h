#pragma once

#include <stdexcept>

#include "napi.h"
#include "xxhash.h"

struct RawSizedArray {
  uint8_t* data;
  size_t length;

  RawSizedArray() : data(nullptr), length(0) {}
  RawSizedArray(uint8_t* data, size_t length) : data(data), length(length) {}
};

class JsValueParseContext {
 private:
  Napi::Env _env;
  const char* _name;
  const char* _entity;
  bool _allowUndefined;

 public:
  JsValueParseContext(Napi::Env env, const char* name, const char* entity,
                      bool allowUndefined = false)
      : _env(env),
        _name(name),
        _entity(entity),
        _allowUndefined(allowUndefined) {}

  [[noreturn]]
  void InvalidType(const char* expectedType) const;

  [[noreturn]]
  void InvalidValue(const char* expectedValue) const;
};

template <typename T>
struct JsValueConverter {
  static T Convert(Napi::Env env, Napi::Value value,
                   const JsValueParseContext& context);

  static Napi::Value ConvertBack(Napi::Env env, T value);
};

template <typename T>
T JsParseArgument(Napi::Env env, Napi::Value value, const char* name) {
  return JsValueConverter<T>::Convert(env, value, {env, name, "parameter"});
}

template <typename T>
T JsParseArgument(Napi::Env env, Napi::Value value, const char* name,
                  T defaultValue) {
  return value.IsUndefined()
             ? defaultValue
             : JsValueConverter<T>::Convert(
                   env, value,
                   {env, name, "parameter", /*allowUndefined = */ true});
}

template <typename T>
T JsParseProperty(Napi::Env env, Napi::Object object, const char* name) {
  Napi::Value propertyValue = object.Get(name);

  return JsValueConverter<T>::Convert(env, propertyValue,
                                      {env, name, "property"});
}

template <typename T>
T JsParseProperty(Napi::Env env, Napi::Object object, const char* name,
                  T defaultValue) {
  Napi::Value propertyValue = object.Get(name);

  if (!propertyValue.IsUndefined()) {
    return JsValueConverter<T>::Convert(
        env, propertyValue,
        {env, name, "property", /*allowUndefined = */ true});
  }

  return defaultValue;
}
