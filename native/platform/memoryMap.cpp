#include "memoryMap.h"

#include <cmath>
#include <memory>
#include <utility>

#include "../helpers.h"
#include "../v8Utils.h"

#if unix
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <dirent.h>
#include <sys/types.h>
#include <unistd.h>
#endif

#include "platformError.h"

#undef min

bool MemoryMappedFile::Open(v8::Isolate* isolate,
                                               const FileOpenOptions& options) {
  size_t offset = options.offset;
  size_t length = options.length;

#ifdef _WIN32
  auto pathBuffer = V8StringToUtf16(isolate, options.path);

  _fileHandle = std::move(FileHandle::OpenRead((LPCWSTR)pathBuffer.get()));
  CHECK_PLATFORM_ERROR(_fileHandle.IsInvalid())

  LARGE_INTEGER largeFileSize;
  CHECK_PLATFORM_ERROR(!GetFileSizeEx(_fileHandle.fd, &largeFileSize))

  size_t fileSize = (size_t)largeFileSize.QuadPart;
  _size = std::min(length, fileSize);

  if (_size == 0) {
    return true;
  }

  _fileMapping =
      CreateFileMappingW(_fileHandle, NULL, PAGE_READONLY, 0, 0, NULL);
  CHECK_PLATFORM_ERROR(_fileMapping == NULL)

  void* mapAddress = MapViewOfFile(_fileMapping, FILE_MAP_READ, 0, 0, 0);

  CHECK_PLATFORM_ERROR(mapAddress == NULL)
#else
  auto pathBuffer = V8StringToUtf8(isolate, options.path);

  FileHandle handle = FileHandle::OpenRead(pathBuffer.get());
  CHECK_PLATFORM_ERROR(handle.IsInvalid())

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