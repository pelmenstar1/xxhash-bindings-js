#include "platformOperationStatus.h"

#define FALLBACK_UNKNOWN                       \
  isolate->ThrowError("Unknown system error"); \
  return

PlatformOperationStatus PlatformOperationStatus::Error() {
#ifdef _WIN32
  DWORD error = GetLastError();
#else
  int error = errno;
#endif

  return {error};
}

void PlatformOperationStatus::ThrowException(v8::Isolate* isolate) {
  if (_error == 0) {
    return;
  }

#ifdef _WIN32
  LPWSTR messageBuffer = nullptr;
  DWORD messageLength = FormatMessageW(
      FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM |
          FORMAT_MESSAGE_IGNORE_INSERTS,
      NULL, _error, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
      reinterpret_cast<LPWSTR>(&messageBuffer), 0, NULL);

  if (messageLength == 0) {
    FALLBACK_UNKNOWN;
  }

  auto messageValue =
      v8::String::NewFromTwoByte(isolate, (uint16_t*)messageBuffer,
                                 v8::NewStringType::kNormal, messageLength);

  LocalFree(messageBuffer);
#else
  const char* errorDesc = strerror(_error);

  if (errorDesc == nullptr) {
    FALLBACK_UNKNOWN;
  }

  auto messageValue = v8::String::NewFromUtf8(isolate, errorDesc);
#endif

  if (messageValue.IsEmpty()) {
    FALLBACK_UNKNOWN;
  } else {
    isolate->ThrowError(messageValue.ToLocalChecked());
  }
}