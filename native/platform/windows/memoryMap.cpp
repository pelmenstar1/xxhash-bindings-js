#ifdef _WIN32

#include "../memoryMap.h"

#include <memory>
#include <iostream>

#include "../../helpers.h"
#include "../../v8Utils.h"

PlatformOperationStatus MemoryMappedFile::Open(v8::Isolate* isolate,
                                               const FileOpenOptions& options) {
  auto pathBuffer = V8StringToUtf16(isolate, options.path);
  _fileHandle =
      CreateFileW((LPCWSTR)pathBuffer.get(), GENERIC_READ, FILE_SHARE_READ,
                  NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

  CHECK_PLATFORM_ERROR(_fileHandle == INVALID_HANDLE_VALUE)

  _fileMapping =
      CreateFileMappingW(_fileHandle, NULL, PAGE_READONLY, 0, 0, NULL);

  CHECK_PLATFORM_ERROR(_fileMapping == NULL)

  size_t offset = options.offset;
  DWORD offsetLow = offset & 0xFFFFFFFF;
  DWORD offsetHigh = offset >> 32;

  LARGE_INTEGER fileSize;
  CHECK_PLATFORM_ERROR(!GetFileSizeEx(_fileHandle, &fileSize));
  _size = min(options.length, (size_t)fileSize.QuadPart);

  _address = (const uint8_t*)MapViewOfFile(_fileMapping, FILE_MAP_READ,
                                           offsetHigh, offsetLow, _size);
  
  CHECK_PLATFORM_ERROR(_address == NULL)

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