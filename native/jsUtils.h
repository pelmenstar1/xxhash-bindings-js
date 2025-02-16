#pragma once

#include <napi.h>

#include "hashers.h"

template <typename CharType>
std::basic_string<CharType> JsStringToCString(Napi::String text);

template <>
inline std::string JsStringToCString<char>(Napi::String text) {
  return text.Utf8Value();
}

template <>
inline std::u16string JsStringToCString<char16_t>(Napi::String text) {
  return text.Utf16Value();
}

inline uint64_t JsParseSeedArgument(Napi::Env env, uint32_t variant,
                                    Napi::Value value) {
  return variant == H32 ? JsParseArgument<uint32_t>(env, value, "seed", 0)
                        : JsParseArgument<uint64_t>(env, value, "seed", 0);
}

inline uint64_t JsParseSeedProperty(Napi::Env env, uint32_t variant,
                                    Napi::Object value) {
  return variant == H32 ? JsParseProperty<uint32_t>(env, value, "seed", 0)
                        : JsParseProperty<uint64_t>(env, value, "seed", 0);
}

inline Napi::Value JsParseHashResult(Napi::Env env, uint32_t variant,
                                     GenericHashResult result) {
  switch (variant) {
    case H32:
      return Napi::Number::New(env, (uint32_t)result.low64);
    case H64:
    case H3:
      return Napi::BigInt::New(env, result.low64);
    case H3_128:
      return JsValueConverter<XXH128_hash_t>::ConvertBack(env, result);
    default:
      return env.Undefined();
  }
}