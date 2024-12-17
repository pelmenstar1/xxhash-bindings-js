#pragma once
#include <v8.h>

#include <cstdint>

struct FileOpenOptions {
  v8::Local<v8::String> path;
  size_t offset;
  size_t length;

  FileOpenOptions(v8::Local<v8::String> path, size_t offset, size_t length)
      : path(path), offset(offset), length(length) {}
};

struct Block {
  uint8_t* data;
  uint32_t length;

  Block(): data(nullptr), length(0) {}
  Block(uint8_t* data, uint32_t length) : data(data), length(length) {}
};
