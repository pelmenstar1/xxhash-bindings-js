#include "../blockReader.h"
#include "../../v8Utils.h"

#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>

const int BUFFER_SIZE = 4096;

BlockReader::~BlockReader() {
  if (_fd != -1) {
    close(_fd);
  }

  if (_buffer != nullptr) {
    free(_buffer);
  } 
}

PlatformOperationStatus BlockReader::Open(v8::Isolate* isolate, v8::Local<v8::String> pathValue) {
  auto pathBuffer = V8StringToUtf8(isolate, pathValue);
  int fd = open(pathBuffer, O_RDONLY);
  if (fd < 0) {
    return PlatformOperationStatus::Error();
  }

  _fd = fd;
  
  int pageSize = getpagesize();
  _buffer = (uint8_t*)aligned_alloc(pageSize, BUFFER_SIZE); 
  if (_buffer == nullptr) {
    return PlatformOperationStatus::Error();
  }

  return PlatformOperationStatus::Success();
}

ReadBlockResult BlockReader::ReadBlock() {
  int result = read(_fd, _buffer, BUFFER_SIZE);
  if (result < 0) {
    return ReadBlockResult::Error();
  }

  return ReadBlockResult::Success(_buffer, (uint32_t)result);
}