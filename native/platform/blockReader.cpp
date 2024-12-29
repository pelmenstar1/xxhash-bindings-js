#include "blockReader.h"

#ifndef _WIN32
#include <fcntl.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>
#endif

#include <cmath>

#include "platformError.h"

#undef min

#ifdef _WIN32

#define ALLOCATE_BUFFER(size) ((uint8_t*)_aligned_malloc(size, 64))
#define FREE_BUFFER _aligned_free

#else

#define ALLOCATE_BUFFER(size) ((uint8_t*)aligned_alloc(64, size))
#define FREE_BUFFER free

#endif

BlockReader::~BlockReader() {
  if (_buffer != nullptr) {
    FREE_BUFFER(_buffer);
  }
}

void BlockReader::Open(const NativeChar* path, size_t offset, size_t length) {
  const uint32_t MAX_BUFFER_SIZE = 4096;

  FileHandle handle = FileHandle::OpenRead(path);
  CHECK_PLATFORM_ERROR(handle.IsInvalid());

#ifdef _WIN32
  if (offset != 0) {
    LARGE_INTEGER largeOffset;
    largeOffset.QuadPart = offset;

    CHECK_PLATFORM_ERROR(
        !SetFilePointerEx(handle, largeOffset, NULL, FILE_BEGIN));
  }

  auto prefBufferSize = (uint32_t)std::min((size_t)MAX_BUFFER_SIZE, length);
#else
  if (offset != 0) {
    CHECK_PLATFORM_ERROR(lseek(handle, offset, SEEK_SET) < 0)
  }

  struct stat fileStat;
  CHECK_PLATFORM_ERROR(fstat(handle, &fileStat) < 0)

  auto prefBufferSize = std::min((size_t)fileStat.st_blksize, length);
#endif

  uint8_t* buffer = _buffer;

  if (buffer == nullptr) {
    buffer = ALLOCATE_BUFFER(prefBufferSize);
    _bufferSize = prefBufferSize;

  } else if (prefBufferSize > _bufferSize) {
    FREE_BUFFER(buffer);

    buffer = ALLOCATE_BUFFER(MAX_BUFFER_SIZE);
    _bufferSize = MAX_BUFFER_SIZE;
  }

  CHECK_PLATFORM_ERROR(buffer == nullptr);

  _buffer = buffer;
  _handle = std::move(handle);

  _offset = 0;
  _length = length;
}

Block BlockReader::ReadBlock() {
  size_t bytesToRead = std::min((size_t)_bufferSize, _length - _offset);

#ifdef _WIN32
  DWORD bytesRead;
  bool result =
      ReadFile(_handle, _buffer, (DWORD)bytesToRead, &bytesRead, NULL);

  if (!result) {
    ThrowPlatformException();
  }
#else
  ssize_t bytesRead = read(_handle, _buffer, bytesToRead);
  if (bytesRead < 0) {
    ThrowPlatformException();
  }
#endif

  _offset += bytesRead;

  return {_buffer, (uint32_t)bytesRead};
}