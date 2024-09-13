#ifndef _PLATFORM_OPERATION_STATUS
#define _PLATFORM_OPERATION_STATUS

#include <v8.h>

#include <cstdint>

#ifdef _WIN32
#include <windows.h>
#endif

class PlatformError {
 private:
#ifdef _WIN32
  DWORD _error;

  PlatformError(DWORD error) : _error(error) {}
#elif defined(unix)
  int _error;

  PlatformError(int error) : _error(error) {}
#endif
 public:
  static inline PlatformError NoError() { return {0}; }
  static PlatformError LastError();

  void ThrowException(v8::Isolate* isolate);
};

class PlatformOperationStatus {
 private:
  bool _success;
  PlatformError _error;

  PlatformOperationStatus(bool success, PlatformError error)
      : _success(success), _error(error) {}

 public:
  static inline PlatformOperationStatus Success() {
    return {true, PlatformError::NoError()};
  }
  static PlatformOperationStatus Error() {
    return {false, PlatformError::LastError()};
  }

  inline bool IsError() { return !_success; }

  void ThrowException(v8::Isolate* isolate) {
    if (!_success) {
      _error.ThrowException(isolate);
    }
  }
};

#endif