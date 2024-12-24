#include "platformError.h"

#include <exception>

#ifdef _WIN32
#include <windows.h>
#endif

#define FALLBACK_UNKNOWN throw std::runtime_error("Unknown system error")

void ThrowPlatformException() {
#ifdef _WIN32
  DWORD error = GetLastError();
#else
  int error = errno;
#endif

#ifdef _WIN32
  LPWSTR messageBuffer = nullptr;
  DWORD messageLength = FormatMessageW(
      FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM |
          FORMAT_MESSAGE_IGNORE_INSERTS,
      NULL, error, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
      reinterpret_cast<LPWSTR>(&messageBuffer), 0, NULL);

  if (messageLength == 0) {
    FALLBACK_UNKNOWN;
  }

  throw PlatformException(messageBuffer, messageLength);
#else
  const char* errorDesc = strerror(error);

  if (errorDesc == nullptr) {
    FALLBACK_UNKNOWN;
  }

  throw PlatformException(errorDesc);
#endif
}

PlatformException::PlatformException(const PlatformException& other) {
#ifdef _WIN32
  _winWhat = (LPWSTR)LocalAlloc(0, other._whatLength * 2);
  _whatLength = other._whatLength;

  memcpy(_winWhat, other._winWhat, other._whatLength * 2);
#else
  _cWhat = other._cWhat;
#endif
}

PlatformException::PlatformException(PlatformException&& other) {
#ifdef _WIN32
  other._winWhat = _winWhat;
  other._whatLength = _whatLength;
#else
  other._cWhat = _cWhat;
#endif
}

PlatformException::~PlatformException() {
#ifdef _WIN32
  LocalFree(_winWhat);
#endif
}
