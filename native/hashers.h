#pragma once

#include <cstdint>
#include <stdexcept>

#include "xxhash.h"

enum HashVariant { H32, H64, H3, H3_128 };

template <int Variant>
struct XxHashTraits {};

template <>
struct XxHashTraits<H32> {
  using State = XXH32_state_t;
  using Seed = uint32_t;
  using Result = uint32_t;

  static State* NewState() { return XXH32_createState(); }
  static void FreeState(State* state) { XXH32_freeState(state); }

  static void Reset(State* state, Seed seed) { XXH32_reset(state, seed); }
  static void Update(State* state, const uint8_t* buffer, size_t length) {
    XXH32_update(state, buffer, length);
  }
  static Result Digest(const State* state) { return XXH32_digest(state); }

  static Result Oneshot(const uint8_t* buffer, size_t length, Seed seed) {
    return XXH32(buffer, length, seed);
  }
};

template <>
struct XxHashTraits<H64> {
  using State = XXH64_state_t;
  using Seed = uint64_t;
  using Result = uint64_t;

  static State* NewState() { return XXH64_createState(); }
  static void FreeState(State* state) { XXH64_freeState(state); }

  static void Reset(State* state, Seed seed) { XXH64_reset(state, seed); }
  static void Update(State* state, const uint8_t* buffer, size_t length) {
    XXH64_update(state, buffer, length);
  }
  static Result Digest(const State* state) { return XXH64_digest(state); }

  static Result Oneshot(const uint8_t* buffer, size_t length, Seed seed) {
    return XXH64(buffer, length, seed);
  }
};

template <>
struct XxHashTraits<H3> {
  using State = XXH3_state_t;
  using Seed = uint64_t;
  using Result = uint64_t;

  static State* NewState() { return XXH3_createState(); }
  static void FreeState(State* state) { XXH3_freeState(state); }

  static void Reset(State* state, Seed seed) {
    XXH3_64bits_reset_withSeed(state, seed);
  }
  static void Update(State* state, const uint8_t* buffer, size_t length) {
    XXH3_64bits_update(state, buffer, length);
  }
  static Result Digest(const State* state) { return XXH3_64bits_digest(state); }

  static Result Oneshot(const uint8_t* buffer, size_t length, Seed seed) {
    return XXH3_64bits_withSeed(buffer, length, seed);
  }
};

template <>
struct XxHashTraits<H3_128> {
  using State = XXH3_state_t;
  using Seed = uint64_t;
  using Result = XXH128_hash_t;

  static State* NewState() { return XXH3_createState(); }
  static void FreeState(State* state) { XXH3_freeState(state); }

  static void Reset(State* state, Seed seed) {
    XXH3_128bits_reset_withSeed(state, seed);
  }
  static void Update(State* state, const uint8_t* buffer, size_t length) {
    XXH3_128bits_update(state, buffer, length);
  }
  static Result Digest(const State* state) {
    return XXH3_128bits_digest(state);
  }

  static Result Oneshot(const uint8_t* buffer, size_t length, Seed seed) {
    return XXH3_128bits_withSeed(buffer, length, seed);
  }
};

template <int Variant>
using XxSeed = typename XxHashTraits<Variant>::Seed;

template <int Variant>
using XxResult = typename XxHashTraits<Variant>::Result;

template <int Variant>
using _XxHashState = typename XxHashTraits<Variant>::State;

template <int Variant>
class XxHashState {
 public:
  XxHashState() {
    _state = Traits::NewState();
    if (_state == nullptr) {
      throw std::runtime_error("Out of memory");
    }
  }

  XxHashState(XxSeed<Variant> seed) : XxHashState<Variant>() { Reset(seed); }

  XxHashState(const XxHashState<Variant>& source) = delete;

  XxHashState(XxHashState<Variant>&& source) {
    _state = source._state;
    source._state = nullptr;
  }

  ~XxHashState() { Traits::FreeState(_state); }

  XxHashState<Variant>& operator=(XxHashState<Variant>&& other) {
    _state = other._state;
    other._state = nullptr;
  }

  void Reset(XxSeed<Variant> seed) { Traits::Reset(_state, seed); }

  void Update(const uint8_t* data, size_t length) {
    Traits::Update(_state, data, length);
  }

  XxResult<Variant> GetResult() const { return Traits::Digest(_state); }

 private:
  using Traits = XxHashTraits<Variant>;

  _XxHashState<Variant>* _state = nullptr;
};

class XxHashDynamicState {
 public:
  virtual ~XxHashDynamicState() {
  }

  virtual void Reset(uint64_t seed) = 0;
  virtual void Update(const uint8_t* data, size_t length) = 0;

  virtual XXH128_hash_t GetResult() const = 0;
};

template <int Variant>
class XxHashDynamicStateImpl : public XxHashDynamicState {
 public:
  XxHashDynamicStateImpl() {
    _state = Traits::NewState();
    if (_state == nullptr) {
      throw std::runtime_error("Out of memory");
    }
  }

  XxHashDynamicStateImpl(uint64_t seed) : XxHashDynamicStateImpl<Variant>() {
    Reset(seed);
  }

  XxHashDynamicStateImpl(const XxHashDynamicStateImpl<Variant>& source) = delete;

  XxHashDynamicStateImpl(XxHashDynamicStateImpl<Variant>&& source) {
    _state = source._state;
    source._state = nullptr;
  }

  virtual ~XxHashDynamicStateImpl() override { Traits::FreeState(_state); }

  XxHashDynamicStateImpl<Variant>& operator=(XxHashDynamicStateImpl<Variant>&& other) {
    _state = other._state;
    other._state = nullptr;
  }

  void Reset(uint64_t seed) override { Traits::Reset(_state, (XxSeed<Variant>)seed); }

  void Update(const uint8_t* data, size_t length) override {
    Traits::Update(_state, data, length);
  }

  XXH128_hash_t GetResult() const override {
    auto digest = Traits::Digest(_state);

    if constexpr (Variant == H3_128) {
      return digest;
    } else {
      XXH128_hash_t result;
      result.low64 = (uint64_t)digest;

      return result;
    }
  }

 private:
  using Traits = XxHashTraits<Variant>;

  _XxHashState<Variant>* _state = nullptr;
};

using XxHashState32 = XxHashDynamicStateImpl<H32>;
using XxHashState64 = XxHashDynamicStateImpl<H64>;
using XxHashState3 = XxHashDynamicStateImpl<H3>;
using XxHashState3_128 = XxHashDynamicStateImpl<H3_128>;