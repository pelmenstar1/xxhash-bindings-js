#ifdef unix

#include "../memoryMap.h"

#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <iostream>
#include <memory>

#include "../../v8Utils.h"

class FileHandle {
 public:
  const int fd;

  FileHandle(int fd) : fd(fd) {}

  ~FileHandle() { close(fd); }
};

PlatformOperationStatus MemoryMappedFile::Open(
    v8::Isolate* isolate, v8::Local<v8::String> pathValue) {
  auto pathBuffer = V8StringToUtf8(isolate, pathValue);
  int fd = open(pathBuffer, O_RDONLY);
  if (fd < 0) {
    return PlatformOperationStatus::Error();
  }

  FileHandle handle(fd);

  struct stat64 statInfo;
  if (fstat64(fd, &statInfo) < 0) {
    return PlatformOperationStatus::Error();
  }

  void* mapAddress =
      mmap(NULL, statInfo.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
  if (mapAddress == nullptr) {
    return PlatformOperationStatus::Error();
  }

  _address = (const uint8_t*)mapAddress;
  _size = statInfo.st_size;

  return PlatformOperationStatus::Success();
}

MemoryMappedFile::~MemoryMappedFile() {
  if (_address != nullptr) {
    munmap((void*)_address, _size);
  }
}

#endif