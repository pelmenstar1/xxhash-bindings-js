#pragma once

#include <napi.h>

template <typename CharType>
std::basic_string<CharType> JsStringToCString(Napi::String text);

template<>
std::string JsStringToCString<char>(Napi::String text) {
  return text.Utf8Value();
}

template<>
std::u16string JsStringToCString<char16_t>(Napi::String text) {
  return text.Utf16Value();
}
