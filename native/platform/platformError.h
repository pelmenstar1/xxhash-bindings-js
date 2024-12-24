#pragma once

#include <v8.h>

#include <stdexcept>
#include <string>

#ifdef _WIN32
#include <windows.h>
#endif

class PlatformException : public std::exception {
 private:
  const char* _cWhat;

#ifdef _WIN32
  LPWSTR _winWhat;
  DWORD _whatLength;
#endif

 public:
#ifdef _WIN32
  PlatformException(LPWSTR winWhat, DWORD whatLength)
      : _cWhat("System error"), _winWhat(winWhat), _whatLength(whatLength) {}
#else
  PlatformException(const char* cWhat) : _cWhat(cWhat) {}
#endif

  PlatformException(const PlatformException& other);
  PlatformException(PlatformException&& other);

  ~PlatformException();

  virtual char const* what() const noexcept override { return _cWhat; }

  v8::Local<v8::String> WhatV8(v8::Isolate* isolate) const;
};

void ThrowPlatformException();

#define CHECK_PLATFORM_ERROR(expr) \
  if (expr) {                      \
    ThrowPlatformException();      \
  }
