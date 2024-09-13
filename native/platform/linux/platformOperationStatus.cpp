#ifdef unix

#include "../platformOperationStatus.h"

PlatformError PlatformError::LastError() {
  int error = errno;

  return {error};
}

void PlatformError::ThrowException(v8::Isolate* isolate) {
  const char* errorDesc = strerror(_error);

  if (errorDesc == nullptr) {
    isolate->ThrowError("Unknown system error");
    return;
  }

  auto messageValue = v8::String::NewFromUtf8(isolate, errorDesc);
  isolate->ThrowError(messageValue.ToLocalChecked());
}

#endif