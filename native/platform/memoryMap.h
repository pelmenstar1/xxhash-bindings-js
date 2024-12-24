#include <v8.h>

#include <cstdint>

#include "types.h"
#include "handle.h"

#ifdef _WIN32
#include <windows.h>
#endif

class MemoryMappedFile {
 public:
  MemoryMappedFile() {}
  MemoryMappedFile(const MemoryMappedFile& other) = delete;
  ~MemoryMappedFile();

  bool Open(v8::Isolate* isolate, const FileOpenOptions& options);

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
  FileHandle _fileHandle;
  HANDLE _fileMapping = INVALID_HANDLE_VALUE;
#endif
};