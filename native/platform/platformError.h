#pragma once

#include <v8.h>

#include <stdexcept>
#include <string>

#ifdef _WIN32
#include <windows.h>

using ErrorDesc = DWORD;
#else
using ErrorDesc = int;
#endif

class PlatformException : public std::exception {
 private:
  ErrorDesc _error;

 public:
  PlatformException(ErrorDesc error) : _error(error) {}

  virtual char const* what() const noexcept override { return "System error"; }

  v8::Local<v8::String> WhatV8(v8::Isolate* isolate) const;

  ErrorDesc ErrorCode() const { return _error; }

  static v8::Local<v8::String> FormatErrorToV8String(v8::Isolate* isolate, ErrorDesc error);
};



[[noreturn]] void ThrowPlatformException();

#define CHECK_PLATFORM_ERROR(expr) \
  if (expr) {                      \
    ThrowPlatformException();      \
  }
