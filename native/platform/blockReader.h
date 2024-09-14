#include <cstdint>

#include "platformOperationStatus.h"
#include "block.h"

#ifdef _WIN32
#include <windows.h>
#endif

class BlockReader {
 public:
  BlockReader() {
  }
  ~BlockReader();

  PlatformOperationStatus Open(v8::Isolate* isolate,
                               v8::Local<v8::String> pathValue);
  ReadBlockResult ReadBlock();

 private:
  uint8_t* _buffer = nullptr;

#ifdef _WIN32
  HANDLE _fileHandle = INVALID_HANDLE_VALUE;
#elif defined(unix)
  int _fd = -1;
#endif
};