#include "jsObjectParser.h"

#include <cmath>

#define CONVERT_DECL(type)                                               \
  template <>                                                            \
  type JsValueConverter<type>::Convert(Napi::Env env, Napi::Value value, \
                                       const JsValueParseContext& context)

#define CONVERT_BACK_DECL(type) \
  template <>                   \
  Napi::Value JsValueConverter<type>::ConvertBack(Napi::Env env, type value)

#define SINGLE_CHECK_CONVERTER(type, check, expectedType) \
  CONVERT_DECL(type) {                         \
    if (value.check()) {                                 \
      return value.As<type>();                            \
    }                                                     \
    context.InvalidType(expectedType);                    \
  }                                                       \
  CONVERT_BACK_DECL(type) { return value; }

SINGLE_CHECK_CONVERTER(Napi::String, IsString, "string")
SINGLE_CHECK_CONVERTER(Napi::Object, IsObject, "object")
SINGLE_CHECK_CONVERTER(Napi::Function, IsFunction, "function")

template <typename UInt, typename Int = std::make_signed_t<UInt>>
UInt ToPositiveIntChecked(Napi::Value value, const JsValueParseContext& context,
                          const char* expectedType) {
  double result = value.As<Napi::Number>().DoubleValue();

  if (std::nearbyint(result) == result) {
    auto intResult = (Int)result;

    if (intResult >= 0) {
      return (UInt)intResult;
    }
  }

  context.InvalidValue(expectedType);
}

CONVERT_DECL(uint32_t) {
  if (value.IsNumber()) {
    return ToPositiveIntChecked<uint32_t>(value, context,
                                          "non-negative integer");
  }

  context.InvalidType("number");
}

CONVERT_BACK_DECL(uint32_t) { return Napi::Number::New(env, value); }

CONVERT_DECL(uint64_t) {
  if (value.IsNumber()) {
    return ToPositiveIntChecked<uint64_t>(value, context,
                                          "non-negative integer or bigint");
  } else if (value.IsBigInt()) {
    auto bigint = value.UnsafeAs<Napi::BigInt>();

    bool lossless = true;
    uint64_t result = bigint.Uint64Value(&lossless);

    if (!lossless) {
      context.InvalidValue("non-negative integer or bigint");
    }

    return result;
  }

  context.InvalidType("number, bigint");
}

CONVERT_BACK_DECL(uint64_t) { return Napi::BigInt::New(env, value); }

CONVERT_DECL(bool) {
  if (value.IsBoolean()) {
    return value.UnsafeAs<Napi::Boolean>().Value();
  }

  context.InvalidType("boolean");
}

CONVERT_BACK_DECL(bool) { return Napi::Boolean::New(env, value); }

CONVERT_DECL(XXH128_hash_t) {
  if (value.IsBigInt()) {
    auto bigint = value.UnsafeAs<Napi::BigInt>();

    int sign;
    size_t wordCount = 2;
    uint64_t words[2];

    bigint.ToWords(&sign, &wordCount, words);
    XXH128_hash_t value;

    switch (wordCount) {
      case 2:
        value.high64 = words[1];
      case 1:
        value.low64 = words[0];
        break;
      default:
        return {};
    }

    return value;
  }

  context.InvalidType("bigint");
}

CONVERT_BACK_DECL(XXH128_hash_t) {
  uint64_t words[2] = {value.low64, value.high64};

  return Napi::BigInt::New(env, 0, 2, words);
}

CONVERT_DECL(RawSizedArray) {
  if (value.IsTypedArray()) {
    auto array = value.UnsafeAs<Napi::TypedArray>();

    if (array.ElementSize() == 1) {
      uint8_t* data = (uint8_t*)array.ArrayBuffer().Data();
      size_t offset = array.ByteOffset();
      size_t length = array.ByteLength();

      return {data + offset, length};
    }
  }

  context.InvalidType("Uint8Array");
}

template <typename... Args>
std::string string_format(size_t maxSize, const char* format, Args... args) {
  std::unique_ptr<char[]> buf(new char[maxSize]);
  int size = std::snprintf(buf.get(), maxSize, format, args...);

  return std::string(buf.get(), size);
}

void JsValueParseContext::InvalidType(const char* expectedType) const {
  size_t bufferLength =
      64 + strlen(_entity) + strlen(_name) + strlen(expectedType);
  const char* undefinedMarker = _allowUndefined ? " or undefined" : "";

  std::string message =
      string_format(bufferLength, "Expected type of the %s \"%s\" is %s%s",
                    _entity, _name, expectedType, undefinedMarker);

  throw Napi::TypeError::New(_env, message);
}

void JsValueParseContext::InvalidValue(const char* expectedValue) const {
  size_t bufferLength =
      64 + strlen(_entity) + strlen(_name) + strlen(expectedValue);

  std::string message =
      string_format(bufferLength, "\"%s\" %s is expected to be %s", _name,
                    _entity, expectedValue);

  throw Napi::TypeError::New(_env, message);
}
