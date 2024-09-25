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

class PlatformOperationStatus {
 private:
  ErrorDescriptor _error;

  PlatformOperationStatus(ErrorDescriptor error) : _error(error) {}

 public:
  static inline PlatformOperationStatus Success() {
    return {0};
  }

  static PlatformOperationStatus Error();

  inline bool IsError() { return _error != 0; }
  inline bool IsSuccess() { return !IsError(); }

  void ThrowException(v8::Isolate* isolate);
};

template<class T>
class PlatformOperationResult {
private:
  PlatformOperationStatus _status;
  T _value;

  PlatformOperationResult(PlatformOperationStatus status, T value) : _status(status), _value(value) {}
public:
  inline bool IsSuccess() { return _status.IsSuccess(); }

  inline T GetValue() { return _value; }

  void ThrowException(v8::Isolate* isolate) {
    _status.ThrowException(isolate);
  }

  static PlatformOperationResult<T> Success(T value) {
    return {PlatformOperationStatus::Success(), value};
  }

  static PlatformOperationResult<T> Error() {
    return {PlatformOperationStatus::Error(), T()};
  }
};

#define CHECK_PLATFORM_ERROR(expr)           \
  if (expr) {                                \
    return PlatformOperationStatus::Error(); \
  }

#endif