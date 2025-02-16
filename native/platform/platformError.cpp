#include "platformError.h"

#include <exception>

#ifdef _WIN32
#include <windows.h>

class LocalBuffer {
 public:
  LocalBuffer(HANDLE handle) : _handle(handle) {}

  ~LocalBuffer() { LocalFree(_handle); }

 private:
  HANDLE _handle;
};
#endif

Napi::String UnknownSystemError(Napi::Env env) {
  return Napi::String::New(env, "Unknown system error");
}

void ThrowPlatformException() {
#ifdef _WIN32
  DWORD error = GetLastError();
#else
  int error = errno;
#endif

  throw PlatformException(error);
}

Napi::String PlatformException::FormatErrorToJsString(Napi::Env env,
                                                      ErrorDesc error) {
#ifdef _WIN32
  LPWSTR messageBuffer = nullptr;
  DWORD messageLength = FormatMessageW(
      FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM |
          FORMAT_MESSAGE_IGNORE_INSERTS,
      NULL, error, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
      reinterpret_cast<LPWSTR>(&messageBuffer), 0, NULL);
  LocalBuffer localBuffer(messageBuffer);

  if (messageLength != 0) {
    return Napi::String::New(env, (const char16_t*)messageBuffer,
                             messageLength);
  }
#else
  const char* errorDesc = strerror(error);

  if (errorDesc != nullptr) {
    return Napi::String::New(env, errorDesc);
  }
#endif

  return UnknownSystemError(env);
}
