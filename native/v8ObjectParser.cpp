#include "v8ObjectParser.h"

#include <cmath>

#define CONVERT_DECL(type)                                         \
  template <>                                                      \
  type V8ValueConverter<type>::Convert(v8::Isolate* isolate,       \
                                       v8::Local<v8::Value> value, \
                                       const V8ValueParseContext& context)

#define CONVERT_BACK_DECL(type)                             \
  template <>                                               \
  v8::Local<v8::Value> V8ValueConverter<type>::ConvertBack( \
      v8::Isolate* isolate, type value)

#define SINGLE_CHECK_CONVERTER(type, check, expectedType) \
  CONVERT_DECL(v8::Local<type>) {                         \
    if (value->check()) {                                 \
      return value.As<type>();                            \
    }                                                     \
    context.InvalidType(expectedType);                    \
  }                                                       \
  CONVERT_BACK_DECL(v8::Local<type>) { return value; }

SINGLE_CHECK_CONVERTER(v8::String, IsString, "string")
SINGLE_CHECK_CONVERTER(v8::Object, IsObject, "object")
SINGLE_CHECK_CONVERTER(v8::Function, IsFunction, "function")

template <typename UInt, typename Int = std::make_signed_t<UInt>>
UInt ToPositiveIntChecked(v8::Local<v8::Value> value,
                          V8ValueParseContext context,
                          const char* expectedType) {
  double result = value.As<v8::Number>()->Value();

  if (std::nearbyint(result) == result) {
    auto intResult = (Int)result;

    if (intResult >= 0) {
      return (UInt)intResult;
    }
  }

  context.InvalidValue(expectedType);
}

CONVERT_DECL(uint32_t) {
  if (value->IsNumber()) {
    return ToPositiveIntChecked<uint32_t>(value, context,
                                          "non-negative integer");
  }

  context.InvalidType("number");
}

CONVERT_BACK_DECL(uint32_t) {
  return v8::Integer::NewFromUnsigned(isolate, value);
}

CONVERT_DECL(uint64_t) {
  if (value->IsNumber()) {
    return ToPositiveIntChecked<uint64_t>(value, context,
                                          "non-negative integer or bigint");
  } else if (value->IsBigInt()) {
    auto bigint = value->ToBigInt(isolate->GetCurrentContext());

    v8::Local<v8::BigInt> bigintValue;
    if (bigint.ToLocal(&bigintValue)) {
      bool lossless = true;
      uint64_t result = bigintValue->Uint64Value(&lossless);

      if (!lossless) {
        context.InvalidValue("non-negative integer or bigint");
      }

      return result;
    }
  }

  context.InvalidType("number, bigint");
}

CONVERT_BACK_DECL(uint64_t) {
  return v8::BigInt::NewFromUnsigned(isolate, value);
}

CONVERT_DECL(bool) {
  if (value->IsBoolean()) {
    return value->BooleanValue(isolate);
  }

  context.InvalidType("boolean");
}

CONVERT_BACK_DECL(bool) { return v8::Boolean::New(isolate, value); }

CONVERT_DECL(XXH128_hash_t) {
  if (value->IsBigInt()) {
    auto bigint = value.As<v8::BigInt>();

    int sign;
    int wordCount = 2;
    uint64_t words[2];

    bigint->ToWordsArray(&sign, &wordCount, words);
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

  return v8::BigInt::NewFromWords(isolate->GetCurrentContext(), 0, 2, words)
      .ToLocalChecked();
}

CONVERT_DECL(RawSizedArray) {
  if (value->IsUint8Array()) {
    auto array = value.As<v8::Uint8Array>();

    uint8_t* data = (uint8_t*)array->Buffer()->Data();
    size_t offset = array->ByteOffset();
    size_t length = array->ByteLength();

    return {data + offset, length};
  }

  context.InvalidType("Uint8Array");
}

template <typename... Args>
std::string string_format(size_t maxSize, const char* format, Args... args) {
  std::unique_ptr<char[]> buf(new char[maxSize]);
  int size = std::snprintf(buf.get(), maxSize, format, args...);

  return std::string(buf.get(), size);
}

void V8ValueParseContext::InvalidType(const char* expectedType) const {
  size_t bufferLength =
      64 + strlen(_entity) + strlen(_name) + strlen(expectedType);
  const char* undefinedMarker = _allowUndefined ? " or undefined" : "";

  std::string message =
      string_format(bufferLength, "Expected type of the %s \"%s\" is %s%s",
                    _entity, _name, expectedType, undefinedMarker);

  throw std::runtime_error(message);
}

void V8ValueParseContext::InvalidValue(const char* expectedValue) const {
  size_t bufferLength =
      64 + strlen(_entity) + strlen(_name) + strlen(expectedValue);

  std::string message =
      string_format(bufferLength, "\"%s\" %s is expected to be %s", _name,
                    _entity, expectedValue);

  throw std::runtime_error(message);
}
