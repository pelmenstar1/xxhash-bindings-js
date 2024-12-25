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

BlockReader::~BlockReader() {
#ifdef _WIN32
  if (_buffer != nullptr) {
    _aligned_free(_buffer);
  }
#else
  if (_buffer != nullptr) {
    free(_buffer);
  }
#endif
}

BlockReader BlockReader::Open(const NativeString& path, size_t offset,
                              size_t length) {
  FileHandle handle = FileHandle::OpenRead(path);
  CHECK_PLATFORM_ERROR(handle.IsInvalid());

#ifdef _WIN32
  if (offset != 0) {
    LARGE_INTEGER largeOffset;
    largeOffset.QuadPart = offset;

    CHECK_PLATFORM_ERROR(
        !SetFilePointerEx(handle, largeOffset, NULL, FILE_BEGIN));
  }

  auto bufferSize = std::min((size_t)4096, length);
  auto buffer = (uint8_t*)_aligned_malloc(bufferSize, 64);
#else
  if (offset != 0) {
    CHECK_PLATFORM_ERROR(lseek(handle, offset, SEEK_SET) < 0)
  }

  struct stat fileStat;
  CHECK_PLATFORM_ERROR(fstat(handle, &fileStat) < 0)

  auto bufferSize = std::min((size_t)fileStat.st_blksize, length);
  auto buffer = (uint8_t*)malloc(bufferSize);
#endif

  CHECK_PLATFORM_ERROR(buffer == nullptr);

  return BlockReader(std::move(handle), buffer, bufferSize, length);
}

Block BlockReader::ReadBlock() {
  size_t bytesToRead = std::min(_bufferSize, _size - _offset);

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