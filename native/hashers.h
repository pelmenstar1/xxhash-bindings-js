#pragma once

#include <cstdint>
#include <optional>

#include "v8Utils.h"
#include "xxhash.h"

enum HashVariant { H32, H64, H3, H3_128 };

template <int Variant>
struct MetaXxHashVariant {
};

#define META(variant, state, seed, result) \
  template <>                              \
  struct MetaXxHashVariant<variant> {      \
    typedef state State;                   \
    typedef seed Seed;                     \
    typedef result Result;                 \
  };

META(H32, XXH32_state_t, uint32_t, uint32_t)
META(H64, XXH64_state_t, uint64_t, uint64_t)
META(H3, XXH3_state_t, uint64_t, uint64_t)
META(H3_128, XXH3_state_t, uint64_t, XXH128_hash_t)

#undef META

template<int Variant> using XxSeed = typename MetaXxHashVariant<Variant>::Seed;
template<int Variant> using XxResult = typename MetaXxHashVariant<Variant>::Result;

template <int Variant>
class XxHashState {
 public:
  XxHashState() {}
  ~XxHashState();

  bool Init(XxSeed<Variant> seed);
  void Update(const uint8_t* data, size_t length);

  XxResult<Variant> GetResult();

 private:
  typename MetaXxHashVariant<Variant>::State* _state = nullptr;
};

template <int Variant>
struct XxHasher {
  static XxResult<Variant> Process(
      v8::Isolate* isolate, const uint8_t* data, size_t length,
      XxSeed<Variant> seed);
};