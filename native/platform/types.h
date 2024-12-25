#pragma once
#include <v8.h>

#include <cstdint>

struct Block {
  uint8_t* data;
  uint32_t length;

  Block(): data(nullptr), length(0) {}
  Block(uint8_t* data, uint32_t length) : data(data), length(length) {}
};
