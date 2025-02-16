#pragma once

#include <napi.h>

#include <stdexcept>
#include <string>

#include "nativeString.h"

#ifdef _WIN32
#include <windows.h>

using ErrorDesc = DWORD;
#else
using ErrorDesc = int;
#endif

class PlatformException : public std::exception {
 public:
  PlatformException(ErrorDesc error) : _error(error) {}

  virtual char const* what() const noexcept override { return "System error"; }

  Napi::String WhatJs(Napi::Env env) const {
    return FormatErrorToJsString(env, _error);
  }
  ErrorDesc ErrorCode() const { return _error; }

  static Napi::String FormatErrorToJsString(Napi::Env env, ErrorDesc error);

 private:
  ErrorDesc _error;
};

[[noreturn]] void ThrowPlatformException();

#define CHECK_PLATFORM_ERROR(expr) \
  if (expr) {                      \
    ThrowPlatformException();      \
  }
