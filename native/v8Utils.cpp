#include "v8Utils.h"

#include <stdexcept>

template <typename CharType>
struct V8StringTraits {};

template <>
struct V8StringTraits<char> {
  static int Length(v8::Isolate* isolate, v8::Local<v8::String> text) {
    return text->Utf8Length(isolate);
  }

  static void Write(v8::Isolate* isolate, v8::Local<v8::String> text,
                    char* buffer, int length) {
    text->WriteUtf8(isolate, buffer, length);
  }

  static v8::MaybeLocal<v8::String> Create(v8::Isolate* isolate,
                                           const char* buffer) {
    return v8::String::NewFromUtf8(isolate, buffer);
  }
};

template <>
struct V8StringTraits<wchar_t> {
  static int Length(v8::Isolate* isolate, v8::Local<v8::String> text) {
    return text->Length();
  }

  static void Write(v8::Isolate* isolate, v8::Local<v8::String> text,
                    wchar_t* buffer, int length) {
    text->Write(isolate, (uint16_t*)buffer, 0, length);
  }

  static v8::MaybeLocal<v8::String> Create(v8::Isolate* isolate,
                                           const wchar_t* buffer) {
    return v8::String::NewFromTwoByte(isolate, (const uint16_t*)buffer);
  }
};

template <typename CharType>
std::basic_string<CharType> V8StringToCString(v8::Isolate* isolate,
                                              v8::Local<v8::String> text) {
  using Traits = V8StringTraits<CharType>;

  int length = Traits::Length(isolate, text);
  auto buffer = new CharType[length + 1];
  Traits::Write(isolate, text, buffer, length);
  buffer[length] = '\0';

  return std::basic_string<CharType>(buffer, length);
}

template <typename CharType>
v8::Local<v8::String> CStringToV8String(v8::Isolate* isolate,
                                        const CharType* text) {
  v8::MaybeLocal<v8::String> maybeResult =
      V8StringTraits<CharType>::Create(isolate, text);

  v8::Local<v8::String> result;
  if (maybeResult.ToLocal(&result)) {
    return result;
  }

  throw std::runtime_error("Cannot create a string");
}

#define IMPL_CHAR_TYPE(type)                                \
  template v8::Local<v8::String> CStringToV8String<type>(   \
      v8::Isolate * isolate, const type* text);             \
  template std::basic_string<type> V8StringToCString<type>( \
      v8::Isolate * isolate, v8::Local<v8::String> text);

IMPL_CHAR_TYPE(char)
IMPL_CHAR_TYPE(wchar_t)

v8::MaybeLocal<v8::Value> V8GetPropertyValue(v8::Isolate* isolate,
                                             v8::Local<v8::Object> object,
                                             const char* name) {
  auto maybeKey = v8::String::NewFromOneByte(isolate, (const uint8_t*)name);
  v8::Local<v8::String> key;

  if (!maybeKey.ToLocal(&key)) {
    throw std::runtime_error("Cannot create a string");
  }

  return object->Get(isolate->GetCurrentContext(), key);
}

void ExecuteCallbackWithErrorOrThrow(v8::Isolate* isolate,
                                     v8::Local<v8::Function> callback,
                                     v8::Local<v8::String> message) {
  if (callback.IsEmpty()) {
    isolate->ThrowError(message);
  } else {
    V8CallFunction(isolate, callback, v8::Exception::Error(message));
  }
}
