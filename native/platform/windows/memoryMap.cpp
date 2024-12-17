#ifdef _WIN32

#include "../memoryMap.h"

#include <iostream>
#include <memory>

#include "../../helpers.h"
#include "../../v8Utils.h"

PlatformOperationStatus MemoryMappedFile::Open(v8::Isolate* isolate,
                                               const FileOpenOptions& options) {
  auto pathBuffer = V8StringToUtf16(isolate, options.path);
  _fileHandle =
      CreateFileW((LPCWSTR)pathBuffer.get(), GENERIC_READ, FILE_SHARE_READ,
                  NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

  CHECK_PLATFORM_ERROR(_fileHandle == INVALID_HANDLE_VALUE)

  LARGE_INTEGER fileSize;
  CHECK_PLATFORM_ERROR(!GetFileSizeEx(_fileHandle, &fileSize))

  size_t totalFileSize = (size_t)fileSize.QuadPart;

  _size = min(options.length, totalFileSize);

  if (_size == 0) {
    return PlatformOperationStatus::Success();
  }

  _fileMapping =
      CreateFileMappingW(_fileHandle, NULL, PAGE_READONLY, 0, 0, NULL);
  CHECK_PLATFORM_ERROR(_fileMapping == NULL)

  _address =
      (const uint8_t*)MapViewOfFile(_fileMapping, FILE_MAP_READ, 0, 0, 0);
  CHECK_PLATFORM_ERROR(_address == NULL)

  size_t offset = options.offset;
  _address += offset;

  if (offset + _size > totalFileSize) {
    _size = (offset >= totalFileSize) ? 0 : (totalFileSize - offset);
  }

  return PlatformOperationStatus::Success();
}

MemoryMappedFile::~MemoryMappedFile() {
  if (_address != nullptr) {
    UnmapViewOfFile(_address);
  }

  if (_fileMapping != INVALID_HANDLE_VALUE) {
    CloseHandle(_fileMapping);
  }

  if (_fileHandle != INVALID_HANDLE_VALUE) {
    CloseHandle(_fileHandle);
  }
}

#endif