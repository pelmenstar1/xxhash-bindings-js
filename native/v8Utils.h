#include <v8.h>
#include <memory>
#include <cstdint>

std::unique_ptr<char[]> V8StringToUtf8(v8::Isolate* isolate, v8::Local<v8::String> text);
std::unique_ptr<uint16_t[]> V8StringToUtf16(v8::Isolate* isolate, v8::Local<v8::String> text);