#ifndef _PLATFORM_BLOCK
#define _PLATFORM_BLOCK

#include <cstdint>

#include "platformOperationStatus.h"

struct Block {
  uint8_t* data;
  uint32_t length;

  Block(): data(nullptr), length(0) {}
  Block(uint8_t* data, uint32_t length) : data(data), length(length) {}
};

/*
class ReadBlockResult {
 private:
  PlatformOperationStatus _status;
  Block _block;

  ReadBlockResult(PlatformOperationStatus status, Block block)
      : _status(status), _block(block) {}

 public:
  inline bool IsSucess() { return _status.IsSuccess(); }

  inline Block GetBlock() { return _block; }

  void ThrowException(v8::Isolate* isolate) {
    _status.ThrowException(isolate);
  }

  static ReadBlockResult Success(uint8_t* data, uint32_t length) {
    return {PlatformOperationStatus::Success(), Block(data, length)};
  }

  static ReadBlockResult Error() {
    return {PlatformOperationStatus::Error(), Block::Empty()};
  }
};
*/

#endif