#include <cstdint>

#include "types.h"
#include "platformOperationStatus.h"

#ifdef _WIN32
#include <windows.h>
#endif

class BlockReader {
 public:
  BlockReader() {}
  ~BlockReader();

  PlatformOperationStatus Open(v8::Isolate* isolate, const FileOpenOptions& options);
  PlatformOperationResult<Block> ReadBlock();

 private:
  uint8_t* _buffer = nullptr;
  size_t _bufferSize = 0;

  size_t _offset = 0;
  size_t _size = 0; 

#ifdef _WIN32
  HANDLE _fileHandle = INVALID_HANDLE_VALUE;
#elif defined(unix)
  int _fileHandle = -1;
#endif
};