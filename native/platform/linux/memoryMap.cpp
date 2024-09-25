#ifdef unix

#include "../memoryMap.h"

#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <cmath>
#include <iostream>
#include <memory>

#include "../../v8Utils.h"

class FileHandle {
 public:
  const int fd;

  FileHandle(int fd) : fd(fd) {}

  ~FileHandle() { close(fd); }
};

PlatformOperationStatus MemoryMappedFile::Open(v8::Isolate* isolate,
                                               const FileOpenOptions& options) {
  auto pathBuffer = V8StringToUtf8(isolate, options.path);

  int fd = open(pathBuffer.get(), O_RDONLY);
  CHECK_PLATFORM_ERROR(fd < 0)

  FileHandle handle(fd);

  size_t offset = options.offset;
  size_t length = options.length;

  if (length == SIZE_MAX) {
    struct stat statInfo;
    CHECK_PLATFORM_ERROR(fstat(fd, &statInfo) < 0)

    length = (ssize_t)statInfo.st_size;

    if (length == 0) {
      // mmap can fail if length == 0.
      // For us, it's ok, MemoryMappedFile will return nullptr pointer and zero length
      // in which case this memory should not be used.
      return PlatformOperationStatus::Success();
    }
  }

  void* mapAddress = mmap(NULL, length, PROT_READ, MAP_PRIVATE, fd, offset);
  CHECK_PLATFORM_ERROR(mapAddress == MAP_FAILED)

  _address = (const uint8_t*)mapAddress;
  _size = length;

  return PlatformOperationStatus::Success();
}

MemoryMappedFile::~MemoryMappedFile() {
  if (_address != nullptr) {
    munmap((void*)_address, _size);
  }
}

#endif