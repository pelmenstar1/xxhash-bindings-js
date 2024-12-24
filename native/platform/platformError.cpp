#include "platformError.h"

#include <exception>

#ifdef _WIN32
#include <windows.h>
#endif

v8::MaybeLocal<v8::String> UnknownSystemError(v8::Isolate* isolate) {
  return v8::String::NewFromUtf8Literal(isolate, "Unknown system error");
}

void ThrowPlatformException() {
#ifdef _WIN32
  DWORD error = GetLastError();
#else
  int error = errno;
#endif

  throw PlatformException(error);
}

v8::Local<v8::String> PlatformException::WhatV8(v8::Isolate* isolate) const {
  v8::MaybeLocal<v8::String> message;

#ifdef _WIN32
  LPWSTR messageBuffer = nullptr;
  DWORD messageLength = FormatMessageW(
      FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM |
          FORMAT_MESSAGE_IGNORE_INSERTS,
      NULL, _error, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
      reinterpret_cast<LPWSTR>(&messageBuffer), 0, NULL);

  if (messageLength == 0) {
    message = UnknownSystemError(isolate);
  } else {
    message =
        v8::String::NewFromTwoByte(isolate, (const uint16_t*)messageBuffer,
                                   v8::NewStringType::kNormal, messageLength);
  }

  LocalFree(messageBuffer);
#else
  const char* errorDesc = strerror(_error);

  if (errorDesc == nullptr) {
    message = UnknownSystemError(isolate);
  } else {
    message = v8::String::NewFromUtf8(isolate, errorDesc);
  }
#endif

  v8::Local<v8::String> messageValue;

  if (message.ToLocal(&messageValue)) {
    return messageValue;
  } else {
    return UnknownSystemError(isolate).ToLocalChecked();
  }
}
