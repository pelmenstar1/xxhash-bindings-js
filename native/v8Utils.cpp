#include "v8Utils.h"

#include <stdexcept>

NativeString V8StringToNative(v8::Isolate* isolate,
                              v8::Local<v8::String> text) {
#ifdef _WIN32
  int length = text->Length();
  auto buffer = new wchar_t[length + 1];
  text->Write(isolate, (uint16_t*)buffer, 0, length);
#else
  int length = text->Utf8Length(isolate);
  auto buffer = new char[length + 1];
  text->WriteUtf8(isolate, buffer);
#endif

  buffer[length] = 0;

  return NativeString(buffer, length);
}

v8::Local<v8::String> V8ConvertNativeString(v8::Isolate* isolate,
                                            const NativeChar* text) {
#ifdef _WIN32
  auto maybeResult = v8::String::NewFromTwoByte(isolate, (const uint16_t*)text);
#else
  auto maybeResult = v8::String::NewFromUtf8(isolate, text);
#endif

  v8::Local<v8::String> result;
  if (maybeResult.ToLocal(&result)) {
    return result;
  }

  throw std::runtime_error("Cannot create a string");
}

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
