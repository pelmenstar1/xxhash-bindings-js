#include <optional>
#include <stdexcept>

#include "v8.h"
#include "v8Utils.h"

template <typename T>
struct V8ValueParser {};

template <>
struct V8ValueParser<v8::Local<v8::String>> {
  std::optional<v8::Local<v8::String>> operator()(v8::Isolate* isolate,
                                                  v8::Local<v8::Value> value) {
    if (!value->IsString()) {
      return {};
    }

    return {value.As<v8::String>()};
  }
};

template <>
struct V8ValueParser<uint32_t> {
  std::optional<uint32_t> operator()(v8::Isolate* isolate,
                                     v8::Local<v8::Value> value,
                                     uint32_t defaultValue) {
    return V8GetUInt32Optional(isolate, value, defaultValue);
  }
};

template <>
struct V8ValueParser<uint64_t> {
  std::optional<uint64_t> operator()(v8::Isolate* isolate,
                                     v8::Local<v8::Value> value,
                                     uint64_t defaultValue) {
    return V8GetUInt64Optional(isolate, value, defaultValue);
  }
};

template <>
struct V8ValueParser<bool> {
  std::optional<bool> operator()(v8::Isolate* isolate,
                                 v8::Local<v8::Value> value,
                                 bool defaultValue) {
    if (value->IsUndefined()) {
      return {defaultValue};
    } else if (value->IsBoolean()) {
      return {value->BooleanValue(isolate)};
    }

    return {};
  }
};

template <typename T>
static std::optional<T> V8ParsePropertyOptional(v8::Isolate* isolate,
                                                v8::Local<v8::Object> object,
                                                const char* name,
                                                T defaultValue) {
  auto property =
      V8GetObjectProperty(isolate->GetCurrentContext(), object, name);
  v8::Local<v8::Value> propertyValue;
  if (property.ToLocal(&propertyValue)) {
    return V8ValueParser<T>()(isolate, propertyValue, defaultValue);
  }

  return {};
}

template <typename T>
static std::optional<T> V8ParseProperty(v8::Isolate* isolate,
                                        v8::Local<v8::Object> object,
                                        const char* name) {
  auto property =
      V8GetObjectProperty(isolate->GetCurrentContext(), object, name);
  v8::Local<v8::Value> propertyValue;
  if (property.ToLocal(&propertyValue)) {
    return V8ValueParser<T>()(isolate, propertyValue);
  }

  return {};
}

#define _V8_PARSE_PROPERTY(isolate, name, expectedType, cType, parse) \
  std::optional<cType> name##Prop__ = parse;                          \
  if (!name##Prop__.has_value()) {                                    \
    throw std::runtime_error("Expected type of property \"" #name     \
                             "\" is " expectedType);                  \
  }                                                                   \
  cType name##Prop = name##Prop__.value();

#define V8_PARSE_PROPERTY(object, name, expectedType, cType) \
  _V8_PARSE_PROPERTY(isolate, name, expectedType, cType,     \
                     V8ParseProperty<cType>(isolate, object, #name))

#define V8_PARSE_PROPERTY_OPTIONAL(object, name, expectedType, cType, \
                                   defaultValue)                      \
  _V8_PARSE_PROPERTY(                                                 \
      isolate, name, expectedType, cType,                             \
      V8ParsePropertyOptional<cType>(isolate, object, #name, defaultValue))
