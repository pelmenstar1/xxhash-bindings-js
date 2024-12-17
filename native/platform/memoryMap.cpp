#include "memoryMap.h"

#include <cmath>
#include <memory>

#include "../helpers.h"
#include "../v8Utils.h"

#if unix
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#endif

#undef min

#if unix
class FileHandle {
 public:
  const int fd;

  FileHandle(int fd) : fd(fd) {}

  ~FileHandle() { close(fd); }
};
#endif

PlatformOperationStatus MemoryMappedFile::Open(v8::Isolate* isolate,
                                               const FileOpenOptions& options) {
  size_t offset = options.offset;
  size_t length = options.length;

#ifdef _WIN32
  auto pathBuffer = V8StringToUtf16(isolate, options.path);
  _fileHandle =
      CreateFileW((LPCWSTR)pathBuffer.get(), GENERIC_READ, FILE_SHARE_READ,
                  NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

  CHECK_PLATFORM_ERROR(_fileHandle == INVALID_HANDLE_VALUE)

  LARGE_INTEGER largeFileSize;
  CHECK_PLATFORM_ERROR(!GetFileSizeEx(_fileHandle, &largeFileSize))

  size_t fileSize = (size_t)largeFileSize.QuadPart;

  _size = std::min(options.length, fileSize);

  if (_size == 0) {
    return PlatformOperationStatus::Success();
  }

  _fileMapping =
      CreateFileMappingW(_fileHandle, NULL, PAGE_READONLY, 0, 0, NULL);
  CHECK_PLATFORM_ERROR(_fileMapping == NULL)

  void* mapAddress = MapViewOfFile(_fileMapping, FILE_MAP_READ, 0, 0, 0);

  CHECK_PLATFORM_ERROR(mapAddress == NULL)
#else
  auto pathBuffer = V8StringToUtf8(isolate, options.path);

  int fd = open(pathBuffer.get(), O_RDONLY);
  CHECK_PLATFORM_ERROR(fd < 0)

  FileHandle handle(fd);

  struct stat statInfo;
  CHECK_PLATFORM_ERROR(fstat(fd, &statInfo) < 0)
  size_t fileSize = (size_t)statInfo.st_size;

  _size = std::min(options.length, fileSize);

  if (_size == 0) {
    // mmap can fail if length == 0.
    // For us, it's ok, MemoryMappedFile will return nullptr pointer and zero
    // length in which case this memory should not be used.
    return PlatformOperationStatus::Success();
  }

  void* mapAddress = mmap(NULL, fileSize, PROT_READ, MAP_PRIVATE, fd, 0);

  CHECK_PLATFORM_ERROR(mapAddress == MAP_FAILED)
#endif
  _address = (const uint8_t*)mapAddress;
  _address += offset;

  if (offset + _size > fileSize) {
    _size = (offset >= fileSize) ? 0 : (fileSize - offset);
  }

  return PlatformOperationStatus::Success();
}

MemoryMappedFile::~MemoryMappedFile() {
#ifdef _WIN32
  if (_address != nullptr) {
    UnmapViewOfFile(_address);
  }

  if (_fileMapping != INVALID_HANDLE_VALUE) {
    CloseHandle(_fileMapping);
  }

  if (_fileHandle != INVALID_HANDLE_VALUE) {
    CloseHandle(_fileHandle);
  }
#else
  if (_address != nullptr) {
    munmap((void*)_address, _size);
  }
#endif
}