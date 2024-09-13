#include "hashers.h"

#include <iostream>

#include "errorMacro.h"
#include "xxhash.h"

uint32_t GetUInt32(v8::Isolate* isolate, V8OptionalSeed optValue) {
  if (optValue.has_value()) {
    auto value = optValue.value();

    if (value->IsNumber()) {
      return value->Uint32Value(isolate->GetCurrentContext()).ToChecked();
    }

    if (value->IsNullOrUndefined()) {
      return 0;
    }

    FATAL_ERROR("Invalid optValue");
  }

  return 0;
}

uint64_t GetUInt64(v8::Isolate* isolate, V8OptionalSeed optValue) {
  if (optValue.has_value()) {
    auto value = optValue.value();

    if (value->IsNumber()) {
      return value->IntegerValue(isolate->GetCurrentContext()).ToChecked();
    }

    if (value->IsBigInt()) {
      return value->ToBigInt(isolate->GetCurrentContext())
          .ToLocalChecked()
          ->Uint64Value();
    }

    if (value->IsNullOrUndefined()) {
      return 0;
    }

    FATAL_ERROR("Invalid optValue");
  }

  return 0;
}

static v8::Local<v8::Value> CreateUInt128Number(v8::Isolate* isolate,
                                                XXH128_hash_t value) {
  uint64_t words[2] = {value.low64, value.high64};

  return v8::BigInt::NewFromWords(isolate->GetCurrentContext(), 0, 2, words)
      .ToLocalChecked();
}

#define IMPLEMENT_STATE(type, create, update, reset, digest, free, getSeed, \
                        factory)                                            \
  template <>                                                               \
  XxHashState<type>::~XxHashState() {                                       \
    free(_stateHolder.state);                                               \
  }                                                                         \
  template <>                                                               \
  bool XxHashState<type>::Init(V8OptionalSeed seed) {                       \
    auto state = create();                                                  \
    if (state == nullptr) return false;                                     \
                                                                            \
    reset(state, getSeed(_isolate, seed));                                  \
    _stateHolder.state = state;                                             \
    return true;                                                            \
  }                                                                         \
  template <>                                                               \
  void XxHashState<type>::Update(const uint8_t* data, size_t length) {      \
    update(_stateHolder.state, data, length);                               \
  }                                                                         \
  template <>                                                               \
  v8::Local<v8::Value> XxHashState<type>::GetResult() {                     \
    return factory(_isolate, digest(_stateHolder.state));                   \
  }

#define X32F(w, name) XXH##w##_##name
#define X3F(w, name) XXH3_##w##bits_##name

#define IMPLEMENT_32_64_STATE(w, getSeed, factoryType)                         \
  IMPLEMENT_STATE(H##w, X32F(w, createState), X32F(w, update), X32F(w, reset), \
                  X32F(w, digest), X32F(w, freeState), getSeed,                \
                  v8::factoryType::NewFromUnsigned)

#define IMPLEMENT_STATE3(name, w, factory)                                \
  IMPLEMENT_STATE(name, XXH3_createState, X3F(w, update),                 \
                  X3F(w, reset_withSeed), X3F(w, digest), XXH3_freeState, \
                  GetUInt64, factory)

IMPLEMENT_32_64_STATE(32, GetUInt32, Integer)
IMPLEMENT_32_64_STATE(64, GetUInt64, BigInt)
IMPLEMENT_STATE3(H3, 64, v8::BigInt::NewFromUnsigned)
IMPLEMENT_STATE3(H3_128, 128, CreateUInt128Number)

#define PROCESS_FUNCTION(type)                                  \
  template <>                                                   \
  v8::Local<v8::Value> XxHasher<type>::Process(                 \
      v8::Isolate* isolate, const uint8_t* data, size_t length, \
      V8OptionalSeed seedValue)

#define IMPLEMENT_PROCESS_32_64(width, factory)                       \
  PROCESS_FUNCTION(H##width) {                                        \
    auto result =                                                     \
        XXH##width(data, length, GetUInt##width(isolate, seedValue)); \
    return factory(isolate, result);                                  \
  }

#define IMPLEMENT_PROCESS_3(type, width, factory)                              \
  PROCESS_FUNCTION(type) {                                                     \
    uint64_t seed = GetUInt64(isolate, seedValue);                             \
    auto result = seed == 0 ? XXH3_##width##bits(data, length)                 \
                            : XXH3_##width##bits_withSeed(data, length, seed); \
    return factory(isolate, result);                                           \
  }

IMPLEMENT_PROCESS_32_64(32, v8::Integer::NewFromUnsigned)
IMPLEMENT_PROCESS_32_64(64, v8::BigInt::NewFromUnsigned)
IMPLEMENT_PROCESS_3(H3, 64, v8::BigInt::NewFromUnsigned)
IMPLEMENT_PROCESS_3(H3_128, 128, CreateUInt128Number)