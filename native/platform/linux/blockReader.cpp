#include "../blockReader.h"

#include <fcntl.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>

#include "../../v8Utils.h"

BlockReader::~BlockReader() {
  if (_fd != -1) {
    close(_fd);
  }

  if (_buffer != nullptr) {
    free(_buffer);
  }
}

PlatformOperationStatus BlockReader::Open(v8::Isolate* isolate,
                                          const FileOpenOptions& options) {
  auto pathBuffer = V8StringToUtf8(isolate, options.path);

  int fd = open(pathBuffer.get(), O_RDONLY);
  CHECK_PLATFORM_ERROR(fd < 0)

  _fd = fd;

  size_t offset = options.offset;
  size_t length = options.length;

  if (offset != 0) {
    CHECK_PLATFORM_ERROR(lseek(fd, offset, SEEK_SET) < 0)
    _offset = offset;
  }

  struct stat fileStat;
  CHECK_PLATFORM_ERROR(fstat(fd, &fileStat) < 0)

  if (length == SIZE_MAX) {
    length = fileStat.st_size;
  } else {
    length = std::min(length, (size_t)fileStat.st_size);
  }

  int pageSize = getpagesize();

  _buffer = (uint8_t*)aligned_alloc(pageSize, fileStat.st_blksize);
  _bufferSize = fileStat.st_blksize;
  _fileSize = length;

  CHECK_PLATFORM_ERROR(_buffer == nullptr)

  return PlatformOperationStatus::Success();
}

ReadBlockResult BlockReader::ReadBlock() {
  size_t readSize = std::min(_fileSize - _offset, _bufferSize);
  ssize_t result = read(_fd, _buffer, readSize);
  if (result < 0) {
    return ReadBlockResult::Error();
  }

  return ReadBlockResult::Success(_buffer, (uint32_t)result);
}