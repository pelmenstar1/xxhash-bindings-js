#ifdef _WIN32

#include "../blockReader.h"

#include "../fileUtils.h"
#include "../../helpers.h"

#include <iostream>
#include <stdlib.h>

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
                                          v8::Local<v8::String> pathValue) {
  HANDLE fileHandle = OpenFileWithV8Path(isolate, pathValue);

  CHECK_PLATFORM_ERROR(fileHandle, INVALID_HANDLE_VALUE)

  _fileHandle = fileHandle;

  SYSTEM_INFO sysInfo;
  GetSystemInfo(&sysInfo);

  _buffer = (uint8_t*)_aligned_malloc(BUFFER_SIZE, sysInfo.dwPageSize);
  if (_buffer == nullptr) {
    return PlatformOperationStatus::Error();
  }

  return PlatformOperationStatus::Success();
}

ReadBlockResult BlockReader::ReadBlock() {
  DWORD bytesRead;
  bool result = ReadFile(_fileHandle, _buffer, BUFFER_SIZE, &bytesRead, NULL);
  if (!result) {
    return ReadBlockResult::Error();
  }

  return ReadBlockResult::Success(_buffer, (uint32_t)bytesRead);
}

#endif