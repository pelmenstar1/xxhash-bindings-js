#ifdef _WIN32

#include "../blockReader.h"

#include <stdlib.h>

#include <iostream>

#include "../../helpers.h"
#include "../../v8Utils.h"

const size_t BUFFER_SIZE = 4096;

BlockReader::~BlockReader() {
  if (_buffer != nullptr) {
    _aligned_free(_buffer);
  }

  if (_fileHandle != INVALID_HANDLE_VALUE) {
    CloseHandle(_fileHandle);
  }
}

PlatformOperationStatus BlockReader::Open(v8::Isolate* isolate,
                                          const FileOpenOptions& options) {
  auto pathBuffer = V8StringToUtf16(isolate, options.path);
  HANDLE fileHandle =
      CreateFileW((LPCWSTR)pathBuffer.get(), GENERIC_READ, FILE_SHARE_READ,
                  NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

  CHECK_PLATFORM_ERROR(fileHandle == INVALID_HANDLE_VALUE)

  _fileHandle = fileHandle;

  size_t offset = options.offset;
  size_t length = options.length;

  if (offset != 0) {
    LARGE_INTEGER largeOffset;
    largeOffset.QuadPart = offset;

    CHECK_PLATFORM_ERROR(!SetFilePointerEx(fileHandle, largeOffset, NULL, FILE_BEGIN));
    _offset = offset;
  }

  LARGE_INTEGER fileSize;
  CHECK_PLATFORM_ERROR(!GetFileSizeEx(_fileHandle, &fileSize));
  _fileSize = min(length, (size_t)fileSize.QuadPart);

  SYSTEM_INFO sysInfo;
  GetSystemInfo(&sysInfo);

  _buffer = (uint8_t*)_aligned_malloc(BUFFER_SIZE, sysInfo.dwPageSize);
  _bufferSize = sysInfo.dwPageSize;

  CHECK_PLATFORM_ERROR(_buffer == nullptr);

  return PlatformOperationStatus::Success();
}

PlatformOperationResult<Block> BlockReader::ReadBlock() {
  DWORD bytesToRead = min(BUFFER_SIZE, _fileSize - _offset);
  DWORD bytesRead;
  bool result = ReadFile(_fileHandle, _buffer, BUFFER_SIZE, &bytesRead, NULL);

  if (!result) {
    return PlatformOperationResult<Block>::Error();
  }

  return PlatformOperationResult<Block>::Success({_buffer, (uint32_t)bytesRead});
}

#endif