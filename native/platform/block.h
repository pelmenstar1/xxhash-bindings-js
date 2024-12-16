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

#endif