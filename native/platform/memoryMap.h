#include <v8.h>

#include <cstdint>

#include "platformOperationStatus.h"
#include "types.h"

#ifdef _WIN32
#include <windows.h>
#endif

class MemoryMapOpenStatus {
 public:
  static MemoryMapOpenStatus Success() {
    return {PlatformOperationStatus::Success(), false};
  }

  static MemoryMapOpenStatus Error() {
    return {PlatformOperationStatus::Error(), false};
  }

  static MemoryMapOpenStatus Incompatible() {
    return {PlatformOperationStatus::Success(), true};
  }

  bool IsSuccess() { return _opStatus.IsSuccess() && !_incompatible; }

  bool IsError() { return _opStatus.IsError(); }

  bool IsIncompatible() { return _incompatible; }

  void ThrowException(v8::Isolate* isolate) { return _opStatus.ThrowException(isolate); }

 private:
  MemoryMapOpenStatus(PlatformOperationStatus opStatus, bool incompatible)
      : _opStatus(opStatus), _incompatible(incompatible) {}

  PlatformOperationStatus _opStatus;
  bool _incompatible;
};

class MemoryMappedFile {
 public:
  MemoryMappedFile() {}
  MemoryMappedFile(const MemoryMappedFile& other) = delete;
  ~MemoryMappedFile();

  MemoryMapOpenStatus Open(v8::Isolate* isolate,
                           const FileOpenOptions& options);

  template <typename Accessor, typename Handler>
  void Access(Accessor acc, Handler handler) {
#ifdef _WIN32
    __try {
      acc(_address);
    } __except (GetExceptionCode() == EXCEPTION_IN_PAGE_ERROR ||
                        GetExceptionCode() == EXCEPTION_ACCESS_VIOLATION
                    ? EXCEPTION_EXECUTE_HANDLER
                    : EXCEPTION_CONTINUE_SEARCH) {
      handler();
    }
#else
    // TODO: Add error handling
    acc(_address);
#endif
  }

  inline size_t GetSize() { return _size; }

 private:
  const uint8_t* _address = nullptr;
  size_t _size = 0;

#ifdef _WIN32
  HANDLE _fileHandle = INVALID_HANDLE_VALUE;
  HANDLE _fileMapping = INVALID_HANDLE_VALUE;
#endif
};