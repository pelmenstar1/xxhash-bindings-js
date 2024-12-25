#pragma once

#include <v8.h>

#include <cstdint>
#include <memory>
#include <optional>

#include "platform/nativeString.h"

typedef std::optional<v8::Local<v8::Value>> V8OptionalValue;

struct RawSizedArray {
  uint8_t* data;
  size_t length;

  RawSizedArray(uint8_t* data, size_t length) : data(data), length(length) {}
};

NativeString V8StringToNative(v8::Isolate* isolate, v8::Local<v8::String> text);

RawSizedArray V8GetBackingStorage(v8::Local<v8::Uint8Array> array);

std::optional<uint32_t> V8GetUInt32Optional(v8::Isolate* isolate,
                                            v8::Local<v8::Value> optValue,
                                            uint32_t ifUndefined = 0);

std::optional<uint64_t> V8GetUInt64Optional(v8::Isolate* isolate,
                                            v8::Local<v8::Value> optValue,
                                            uint64_t ifUndefined = 0);

v8::Local<v8::Value> V8CreateUInt128Number(v8::Isolate* isolate, uint64_t low,
                                           uint64_t high);

v8::MaybeLocal<v8::Value> V8GetObjectProperty(v8::Local<v8::Context> context,
                                              v8::Local<v8::Object> obj,
                                              const char* name);