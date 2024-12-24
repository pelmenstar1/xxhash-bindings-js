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

BlockReader BlockReader::Open(v8::Isolate* isolate,
                              const FileOpenOptions& options) {
  size_t offset = options.offset;
  size_t length = options.length;

#ifdef _WIN32
  auto pathBuffer = V8StringToUtf16(isolate, options.path);
  FileHandle handle = FileHandle(
      CreateFileW((LPCWSTR)pathBuffer.get(), GENERIC_READ, FILE_SHARE_READ,
                  NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL));

  CHECK_PLATFORM_ERROR(handle.fd == INVALID_HANDLE_VALUE)

  if (offset != 0) {
    LARGE_INTEGER largeOffset;
    largeOffset.QuadPart = offset;

    CHECK_PLATFORM_ERROR(
        !SetFilePointerEx(handle.fd, largeOffset, NULL, FILE_BEGIN));
  }

  auto bufferSize = std::min((size_t)4096, length);
  auto buffer = (uint8_t*)_aligned_malloc(bufferSize, 64);
#else
  auto pathBuffer = V8StringToUtf8(isolate, options.path);

  FileHandle handle = FileHandle(open(pathBuffer.get(), O_RDONLY));
  CHECK_PLATFORM_ERROR(handle.fd < 0)

  if (offset != 0) {
    CHECK_PLATFORM_ERROR(lseek(handle.fd, offset, SEEK_SET) < 0)
  }

  struct stat fileStat;
  CHECK_PLATFORM_ERROR(fstat(handle.fd, &fileStat) < 0)

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
      ReadFile(_handle.fd, _buffer, (DWORD)bytesToRead, &bytesRead, NULL);

  if (!result) {
    ThrowPlatformException();
  }
#else
  ssize_t bytesRead = read(_handle.fd, _buffer, bytesToRead);
  if (bytesRead < 0) {
    ThrowPlatformException();
  }
#endif

  _offset += bytesRead;

  return {_buffer, (uint32_t)bytesRead};
}