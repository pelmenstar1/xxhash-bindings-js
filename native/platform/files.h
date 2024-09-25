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