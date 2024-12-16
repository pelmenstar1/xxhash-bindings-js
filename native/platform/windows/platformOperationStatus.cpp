#ifdef _WIN32

#include "../platformOperationStatus.h"

PlatformOperationStatus PlatformOperationStatus::Error() {
  DWORD lastError = GetLastError();

  return {lastError};
}

void PlatformOperationStatus::ThrowException(v8::Isolate* isolate) {
  if (_error == 0) {
    return;
  }

  LPWSTR messageBuffer = nullptr;
  DWORD messageLength = FormatMessageW(
      FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM |
          FORMAT_MESSAGE_IGNORE_INSERTS,
      NULL, _error, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
      reinterpret_cast<LPWSTR>(&messageBuffer), 0, NULL);

  if (messageLength == 0) {
    isolate->ThrowError("Unknown system error");
    return;
  }

  auto messageValue =
      v8::String::NewFromTwoByte(isolate, (uint16_t*)messageBuffer,
                                 v8::NewStringType::kNormal, messageLength);
  
  if (!messageValue.IsEmpty()) {
    isolate->ThrowError(messageValue.ToLocalChecked());
  }

  LocalFree(messageBuffer);
} 

#endif