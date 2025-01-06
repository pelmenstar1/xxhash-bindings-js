#pragma once

#include <v8.h>
#include "platform/nativeString.h"

template <typename CharType>
std::basic_string<CharType> V8StringToCString(v8::Isolate* isolate,
                                              v8::Local<v8::String> text);

template<typename CharType>
v8::Local<v8::String> CStringToV8String(v8::Isolate* isolate, const CharType* text);

template<typename CharType>
v8::Local<v8::String> CStringToV8String(v8::Isolate* isolate, const std::basic_string<CharType>& text) {
  return CStringToV8String(isolate, text.c_str());
}

v8::MaybeLocal<v8::Value> V8GetPropertyValue(v8::Isolate* isolate,
                                             v8::Local<v8::Object> object,
                                             const char* name);

template <typename... Args>
v8::MaybeLocal<v8::Value> V8CallFunction(v8::Isolate* isolate,
                                         v8::Local<v8::Function> fn,
                                         Args... args) {
  constexpr int argc = sizeof...(Args);
  v8::Local<v8::Value> argv[argc] = {args...};

  return fn->Call(isolate->GetCurrentContext(), v8::Undefined(isolate), argc,
                  argv);
}

void ExecuteCallbackWithErrorOrThrow(v8::Isolate* isolate,
                                     v8::Local<v8::Function> callback,
                                     v8::Local<v8::String> message);