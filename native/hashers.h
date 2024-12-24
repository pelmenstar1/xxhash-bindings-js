#pragma once

#include <cstdint>
#include <optional>

#include "v8Utils.h"
#include "xxhash.h"

enum HashVariant { H32, H64, H3, H3_128 };

template <int Variant>
struct MetaXxHashVariant {};

template <>
struct MetaXxHashVariant<H32> {
  using State = XXH32_state_t;
  using Seed = uint32_t;
  using Result = uint32_t;
};

template <>
struct MetaXxHashVariant<H64> {
  using State = XXH64_state_t;
  using Seed = uint64_t;
  using Result = uint64_t;
};

template <>
struct MetaXxHashVariant<H3> {
  using State = XXH3_state_t;
  using Seed = uint64_t;
  using Result = uint64_t;
};

template <>
struct MetaXxHashVariant<H3_128> {
  using State = XXH3_state_t;
  using Seed = uint64_t;
  using Result = XXH128_hash_t;
};

template <int Variant>
using XxSeed = typename MetaXxHashVariant<Variant>::Seed;

template <int Variant>
using XxResult = typename MetaXxHashVariant<Variant>::Result;

template <int Variant>
class XxHashState {
 public:
  XxHashState() {}
  XxHashState(const XxHashState<Variant>& source) = delete;

  XxHashState(XxHashState<Variant>&& source) {
    _state = source._state;
    source._state = nullptr;
  }

  ~XxHashState();

  void Init(XxSeed<Variant> seed);
  void Update(const uint8_t* data, size_t length);

  XxResult<Variant> GetResult();

 private:
  typename MetaXxHashVariant<Variant>::State* _state = nullptr;
};

template <int Variant>
struct XxHasher {
  static XxResult<Variant> Process(v8::Isolate* isolate, const uint8_t* data,
                                   size_t length, XxSeed<Variant> seed);
};