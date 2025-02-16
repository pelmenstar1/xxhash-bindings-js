#pragma once

#include <uv.h>

#include <cstdint>

#ifdef _WIN32
#include <windows.h>
#endif

#include "handle.h"
#include "nativeString.h"

struct Block {
  uint8_t* data;
  size_t length;

  Block() : data(nullptr), length(0) {}
  Block(uint8_t* data, size_t length) : data(data), length(length) {}
};

// Reusable file reader - it can change the file it's
// reading without the re-allocation of buffers.
class BlockReader {
 public:
  BlockReader() {}
  ~BlockReader();

  void Open(const NativeString& path, size_t offset, size_t length);

  Block ReadBlock();

 private:
  FileHandle _handle;

  uint8_t* _buffer = nullptr;
  uint32_t _bufferSize = 0;

  size_t _offset = 0;
  size_t _length = 0;
};

class AsyncBlockReader {
 public:
  AsyncBlockReader(size_t fileOffset, size_t length)
      : _fileOffset(fileOffset), _length(length) {
    _readFromBeginning = fileOffset == 0;
  }
  
  virtual ~AsyncBlockReader();

  virtual void OnBlock(const uint8_t* data, size_t length) = 0;
  virtual void OnError(const char* message) = 0;
  virtual void OnEnd() = 0;

  // Schedules the reading of the file to uv loop.
  // path is copied internally, so it is not required to exist after this call.
  void Schedule(uv_loop_t* loop, const char* path);

  bool _HandleReadResponse(uv_fs_t* req);
  bool _HandleOpenResponse(uv_fs_t* req);

 private:
  static constexpr size_t BufferSize = 4096;

  size_t _fileOffset;
  size_t _bytesRead = 0;
  size_t _length;

  uv_fs_t _openReq;
  uv_fs_t _readReq;

  uv_buf_t _uvBuffer;

  bool _readFromBeginning;
  bool _readInitialized = false;
  bool _openInitialized = false;

  void HandleError(int errorCode);
};
