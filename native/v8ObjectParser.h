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

class V8ValueParseContext {
 private:
  const char* _name;
  const char* _entity;
  bool _allowUndefined;

 public:
  V8ValueParseContext(const char* name, const char* entity,
                      bool allowUndefined = false)
      : _name(name), _entity(entity), _allowUndefined(allowUndefined) {}

  [[noreturn]]
  void InvalidType(const char* expectedType) const;

  [[noreturn]]
  void InvalidValue(const char* expectedValue) const;
};

template <typename T>
struct V8ValueConverter {
  static T Convert(v8::Isolate* isolate, v8::Local<v8::Value> value,
                   const V8ValueParseContext& context);
              
  static v8::Local<v8::Value> ConvertBack(v8::Isolate* isolate, T value);
};

template <typename T>
T V8ParseArgument(v8::Isolate* isolate, v8::Local<v8::Value> value,
                  const char* name) {
  return V8ValueConverter<T>::Convert(isolate, value, {name, "parameter"});
}

template <typename T>
T V8ParseArgument(v8::Isolate* isolate, v8::Local<v8::Value> value,
                  const char* name, T defaultValue) {
  return value->IsUndefined()
             ? defaultValue
             : V8ValueConverter<T>::Convert(
                   isolate, value,
                   {name, "parameter", /*allowUndefined = */ true});
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
    return V8ValueConverter<T>::Convert(
        isolate, propertyValue, {name, "property", /*allowUndefined = */ true});
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

