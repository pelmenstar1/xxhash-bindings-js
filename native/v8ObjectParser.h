#pragma once

#include <stdexcept>

#include "v8.h"
#include "v8Utils.h"
#include "xxhash.h"

struct RawSizedArray {
  uint8_t* data;
  size_t length;

  RawSizedArray() : data(nullptr), length(0) {}
  RawSizedArray(uint8_t* data, size_t length) : data(data), length(length) {}
};

template <typename T>
struct V8ValueConverter {};

class V8ValueParseContext {
 private:
  const char* _name;
  const char* _entity;

 public:
  V8ValueParseContext(const char* name, const char* entity)
      : _name(name), _entity(entity) {}

  [[noreturn]]
  void InvalidType(const char* expectedType) const;
};

template <typename T>
T V8ParseArgument(v8::Isolate* isolate, v8::Local<v8::Value> value,
                  const char* name) {
  return V8ValueConverter<T>::Convert(isolate, value, {name, "parameter"});
}

template <typename T>
T V8ParseArgument(v8::Isolate* isolate, v8::Local<v8::Value> value,
                  const char* name, T defaultValue) {
  return value->IsUndefined() ? defaultValue
                              : V8ParseArgument<T>(isolate, value, name);
}

template <typename T>
T V8ParseProperty(v8::Isolate* isolate, v8::Local<v8::Object> object,
                  const char* name) {
  auto property = V8GetPropertyValue(isolate, object, name);

  v8::Local<v8::Value> propertyValue;
  if (property.ToLocal(&propertyValue)) {
    return V8ValueConverter<T>::Convert(isolate, propertyValue,
                                        {name, "property"});
  }

  throw std::runtime_error("Property is required");
}

template <typename T>
T V8ParseProperty(v8::Isolate* isolate, v8::Local<v8::Object> object,
                  const char* name, T defaultValue) {
  auto property = V8GetPropertyValue(isolate, object, name);

  v8::Local<v8::Value> propertyValue;
  if (property.ToLocal(&propertyValue) && !propertyValue->IsUndefined()) {
    return V8ValueConverter<T>::Convert(isolate, propertyValue,
                                        {name, "property"});
  }

  return defaultValue;
}

#ifdef _MSC_VER
#define CROSS_VA_COMMA(...) , __VA_ARGS__
#else
#define CROSS_VA_COMMA(...) __VA_OPT__(, ) __VA_ARGS__
#endif

#define V8_PARSE_PROPERTY(object, name, cType, ...)          \
  cType name##Prop = V8ParseProperty<cType>(isolate, object, \
                                            #name CROSS_VA_COMMA(__VA_ARGS__))

#define V8_PARSE_ARGUMENT(name, index, cType, ...)    \
  name = V8ParseArgument<cType>(isolate, info[index], \
                                #name CROSS_VA_COMMA(__VA_ARGS__))

#define DEFINE_CONVERTER(type)                                                 \
  template <>                                                                  \
  struct V8ValueConverter<type> {                                              \
    static type Convert(v8::Isolate* isolate, v8::Local<v8::Value> value,      \
                        const V8ValueParseContext& context);                   \
    static v8::Local<v8::Value> ConvertBack(v8::Isolate* isolate, type value); \
  };

DEFINE_CONVERTER(v8::Local<v8::String>)
DEFINE_CONVERTER(v8::Local<v8::Object>)
DEFINE_CONVERTER(uint32_t)
DEFINE_CONVERTER(uint64_t)
DEFINE_CONVERTER(bool)
DEFINE_CONVERTER(XXH128_hash_t)
DEFINE_CONVERTER(RawSizedArray)

#undef DEFINE_CONVERTER
