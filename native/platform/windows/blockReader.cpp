#ifdef _WIN32

#include "../blockReader.h"

#include "../../helpers.h"
#include "../../v8Utils.h"

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
 
  if (offset != 0) {
    LARGE_INTEGER largeOffset;
    largeOffset.QuadPart = offset;

    CHECK_PLATFORM_ERROR(!SetFilePointerEx(fileHandle, largeOffset, NULL, FILE_BEGIN));
  }

  _size = options.length;
  _offset = 0;

  _bufferSize = min(4096, _size);
  _buffer = (uint8_t*)_aligned_malloc(_bufferSize, 64);

  CHECK_PLATFORM_ERROR(_buffer == nullptr);

  return PlatformOperationStatus::Success();
}

PlatformOperationResult<Block> BlockReader::ReadBlock() {
  DWORD bytesToRead = min(_bufferSize, _size - _offset);
  DWORD bytesRead;
  bool result = ReadFile(_fileHandle, _buffer, bytesToRead, &bytesRead, NULL);

  if (!result) {
    return PlatformOperationResult<Block>::Error();
  }

  _offset += bytesRead;

  return PlatformOperationResult<Block>::Success({_buffer, (uint32_t)bytesRead});
}

#endif