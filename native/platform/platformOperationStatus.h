#ifndef _PLATFORM_OPERATION_STATUS
#define _PLATFORM_OPERATION_STATUS

#include <v8.h>

#include <cstdint>

#ifdef _WIN32
#include <windows.h>

typedef DWORD ErrorDescriptor;
#elif defined(unix)
typedef int ErrorDescriptor;
#endif

class PlatformError {
 private:
  ErrorDescriptor _error;

  PlatformError(ErrorDescriptor error) : _error(error) {
  }
 public:
  static inline PlatformError NoError() { return {0}; }
  static PlatformError LastError();

  inline bool IsActualError() {
    return _error != 0;
  }

  void ThrowException(v8::Isolate* isolate);
};

class PlatformOperationStatus {
 private:
  PlatformError _error;

  PlatformOperationStatus(PlatformError error) : _error(error) {}

 public:
  static inline PlatformOperationStatus Success() {
    return {PlatformError::NoError()};
  }
  static PlatformOperationStatus Error() {
    return {PlatformError::LastError()};
  }

  inline bool IsError() { return _error.IsActualError(); }
  inline bool IsSuccess() { return !IsError(); }

  void ThrowException(v8::Isolate* isolate) {
    if (_error.IsActualError()) {
      _error.ThrowException(isolate);
    }
  }
};

#endif