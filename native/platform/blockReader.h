#pragma once

#include <cstdint>

#ifdef _WIN32
#include <windows.h>
#endif

#include "handle.h"
#include "nativeString.h"

struct Block {
  uint8_t* data;
  size_t length;

  Block(): data(nullptr), length(0) {}
  Block(uint8_t* data, size_t length) : data(data), length(length) {}
};

// Reusable file reader - it can change the file it's 
// reading without the realloaction of buffers.
class BlockReader {
 public:
  BlockReader() {}
  ~BlockReader();

  void Open(const NativeChar* path, size_t offset, size_t length);

  Block ReadBlock();
 private:
  FileHandle _handle;

  uint8_t* _buffer = nullptr;
  uint32_t _bufferSize = 0;

  size_t _offset = 0;
  size_t _length = 0;
};