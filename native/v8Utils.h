#include <v8.h>
#include <memory>
#include <cstdint>
#include <optional>

typedef std::optional<v8::Local<v8::Value>> V8OptionalValue;  

std::unique_ptr<char[]> V8StringToUtf8(v8::Isolate* isolate, v8::Local<v8::String> text);
std::unique_ptr<uint16_t[]> V8StringToUtf16(v8::Isolate* isolate, v8::Local<v8::String> text);

std::optional<uint32_t> V8GetUInt32Optional(v8::Isolate* isolate, v8::Local<v8::Value> optValue);
std::optional<uint64_t> V8GetUInt64Optional(v8::Isolate* isolate, v8::Local<v8::Value> optValue);
v8::Local<v8::Value> V8CreateUInt128Number(v8::Isolate* isolate, uint64_t low, uint64_t high);