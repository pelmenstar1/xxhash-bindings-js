#include "blockReader.h"

#include "../helpers.h"
#include "../v8Utils.h"

#ifndef _WIN32
#include <fcntl.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>
#endif

#include <cmath>

#undef min

BlockReader::~BlockReader() {
#ifdef _WIN32
  if (_buffer != nullptr) {
    _aligned_free(_buffer);
  }

  if (_fileHandle != INVALID_HANDLE_VALUE) {
    CloseHandle(_fileHandle);
  }
#else
  if (_fileHandle != -1) {
    close(_fileHandle);
  }

  if (_buffer != nullptr) {
    free(_buffer);
  }
#endif
}

PlatformOperationStatus BlockReader::Open(v8::Isolate* isolate,
                                          const FileOpenOptions& options) {
  size_t offset = options.offset;
  size_t length = options.length;

  _offset = 0;
  _size = length;

#ifdef _WIN32
  auto pathBuffer = V8StringToUtf16(isolate, options.path);
  HANDLE fileHandle =
      CreateFileW((LPCWSTR)pathBuffer.get(), GENERIC_READ, FILE_SHARE_READ,
                  NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

  CHECK_PLATFORM_ERROR(fileHandle == INVALID_HANDLE_VALUE)

  _fileHandle = fileHandle;

  if (offset != 0) {
    LARGE_INTEGER largeOffset;
    largeOffset.QuadPart = offset;

    CHECK_PLATFORM_ERROR(
        !SetFilePointerEx(fileHandle, largeOffset, NULL, FILE_BEGIN));
  }

  _bufferSize = std::min((size_t)4096, _size);
  _buffer = (uint8_t*)_aligned_malloc(_bufferSize, 64);
#else
  auto pathBuffer = V8StringToUtf8(isolate, options.path);

  int fd = open(pathBuffer.get(), O_RDONLY);
  CHECK_PLATFORM_ERROR(fd < 0)

  _fileHandle = fd;

  if (offset != 0) {
    CHECK_PLATFORM_ERROR(lseek(fd, offset, SEEK_SET) < 0)
  }

  struct stat fileStat;
  CHECK_PLATFORM_ERROR(fstat(fd, &fileStat) < 0)

  _bufferSize = std::min((size_t)fileStat.st_blksize, length);
  _buffer = (uint8_t*)malloc(_bufferSize);
#endif

  CHECK_PLATFORM_ERROR(_buffer == nullptr);

  return PlatformOperationStatus::Success();
}

PlatformOperationResult<Block> BlockReader::ReadBlock() {
  size_t bytesToRead = std::min(_bufferSize, _size - _offset);

#ifdef _WIN32
  DWORD bytesRead;
  bool result =
      ReadFile(_fileHandle, _buffer, (DWORD)bytesToRead, &bytesRead, NULL);

  if (!result) {
    return PlatformOperationResult<Block>::Error();
  }
#else
  ssize_t bytesRead = read(_fileHandle, _buffer, bytesToRead);
  if (bytesRead < 0) {
    return PlatformOperationResult<Block>::Error();
  }
#endif

  _offset += bytesRead;

  return PlatformOperationResult<Block>::Success(
      {_buffer, (uint32_t)bytesRead});
}