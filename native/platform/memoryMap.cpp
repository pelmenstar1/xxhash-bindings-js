#include "memoryMap.h"

#include <cmath>
#include <memory>
#include <utility>

#if defined(__linux__) || defined(__APPLE__)
#include <dirent.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#endif

#include "platformError.h"

#undef min

bool MemoryMappedFile::Open(const NativeChar* path, size_t offset,
                            size_t length) {
  auto handle = FileHandle::OpenRead(path);
  CHECK_PLATFORM_ERROR(handle.IsInvalid())

#ifdef _WIN32
  LARGE_INTEGER largeFileSize;
  CHECK_PLATFORM_ERROR(!GetFileSizeEx(handle, &largeFileSize))

  size_t fileSize = (size_t)largeFileSize.QuadPart;
  _size = std::min(length, fileSize);

  if (_size == 0) {
    return true;
  }

  _fileMapping = CreateFileMappingW(handle, NULL, PAGE_READONLY, 0, 0, NULL);
  CHECK_PLATFORM_ERROR(_fileMapping == NULL)

  void* mapAddress = MapViewOfFile(_fileMapping, FILE_MAP_READ, 0, 0, 0);

  CHECK_PLATFORM_ERROR(mapAddress == NULL)

  _fileHandle = std::move(handle);
#else
  struct stat statInfo;
  CHECK_PLATFORM_ERROR(fstat(handle, &statInfo) < 0)

  if (!S_ISREG(statInfo.st_mode)) {
    // Avoid using mmap on non-regular files.
    return false;
  }

  size_t fileSize = (size_t)statInfo.st_size;
  _size = std::min(length, fileSize);

  if (_size == 0) {
    // mmap can fail if fileSize == 0.
    // For us, it's ok, MemoryMappedFile will return nullptr pointer and zero
    // length in which case this memory should not be used.
    return true;
  }

  void* mapAddress = mmap(NULL, fileSize, PROT_READ, MAP_PRIVATE, handle, 0);

  CHECK_PLATFORM_ERROR(mapAddress == MAP_FAILED)
#endif
  _address = (const uint8_t*)mapAddress;
  _address += offset;

  if (offset + _size > fileSize) {
    _size = (offset >= fileSize) ? 0 : (fileSize - offset);
  }

  return true;
}

MemoryMappedFile::~MemoryMappedFile() {
#ifdef _WIN32
  if (_address != nullptr) {
    UnmapViewOfFile(_address);
  }

  if (_fileMapping != INVALID_HANDLE_VALUE) {
    CloseHandle(_fileMapping);
  }
#else
  if (_address != nullptr) {
    munmap((void*)_address, _size);
  }
#endif
}