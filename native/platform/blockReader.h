#include <cstdint>

#ifdef _WIN32
#include <windows.h>
#endif

#include "types.h"
#include "handle.h"
#include "nativeString.h"

class BlockReader {
 public:
  ~BlockReader();

  static BlockReader Open(const NativeString& path, size_t offset, size_t length);
  Block ReadBlock();

 private:
  BlockReader(FileHandle handle, uint8_t* buffer, size_t bufferSize,
              size_t size)
      : _handle(std::move(handle)),
        _buffer(buffer),
        _bufferSize(bufferSize),
        _offset(0),
        _size(size) {}

  FileHandle _handle;

  uint8_t* _buffer;
  size_t _bufferSize;

  size_t _offset;
  size_t _size;
};