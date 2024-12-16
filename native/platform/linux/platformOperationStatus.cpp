#ifdef unix

#include "../platformOperationStatus.h"

PlatformOperationStatus PlatformOperationStatus::Error() {
  int error = errno;

  return {error};
}

void PlatformOperationStatus::ThrowException(v8::Isolate* isolate) {
  if (_error == 0) {
    return;
  }

  const char* errorDesc = strerror(_error);

  if (errorDesc == nullptr) {
    isolate->ThrowError("Unknown system error");
    return;
  }

  auto messageValue = v8::String::NewFromUtf8(isolate, errorDesc);

  if (!messageValue.IsEmpty()) {
    isolate->ThrowError(messageValue.ToLocalChecked());
  }
}

#endif