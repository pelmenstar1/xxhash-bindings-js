#include <cstdint>

#include "block.h"
#include "files.h"
#include "platformOperationStatus.h"

#ifdef _WIN32
#include <windows.h>
#endif

class BlockReader {
 public:
  BlockReader() {}
  ~BlockReader();

  PlatformOperationStatus Open(v8::Isolate* isolate, const FileOpenOptions& options);
  ReadBlockResult ReadBlock();

 private:
  uint8_t* _buffer = nullptr;
  size_t _bufferSize = 0;

  size_t _offset = 0;
  size_t _fileSize = 0; 

#ifdef _WIN32
  HANDLE _fileHandle = INVALID_HANDLE_VALUE;
#elif defined(unix)
  int _fd = -1;
#endif
};