#pragma once

#include <cstdint>
#include <stdexcept>

#include "xxhash.h"

enum HashVariant { H32, H64, H3, H3_128 };

constexpr uint32_t HASH_VARIANTS_COUNT = 4;

class GenericHashResult {
 public:
  GenericHashResult() {
    _value.low64 = 0;
    _value.high64 = 0;
  }
  GenericHashResult(uint64_t value) {
    _value.low64 = value;
    _value.high64 = 0;
  }
  GenericHashResult(XXH128_hash_t value) : _value(value) {}

  explicit operator uint64_t() const { return _value.low64; }

  operator XXH128_hash_t() const { return _value; }

  XXH128_hash_t Value() const { return _value; }

 private:
  XXH128_hash_t _value;
};

class XxHashDynamicState {
 public:
  XxHashDynamicState() : _variant(0), _state(nullptr) {}

  XxHashDynamicState(uint32_t variant) : _variant(variant) {
    switch (variant) {
      case H32:
        _state = XXH32_createState();
        break;
      case H64:
        _state = XXH64_createState();
        break;
      case H3:
      case H3_128:
        _state = XXH3_createState();
        break;
    }

    if (_state == nullptr) {
      throw std::runtime_error("Out of memory");
    }
  }

  XxHashDynamicState(uint32_t variant, uint64_t seed)
      : XxHashDynamicState(variant) {
    Reset(seed);
  }

  XxHashDynamicState(const XxHashDynamicState& source) = delete;

  XxHashDynamicState(XxHashDynamicState&& source) {
    _variant = source._variant;
    _state = source._state;
    source._state = nullptr;
  }

  ~XxHashDynamicState() {
    if (_state != nullptr) {
      switch (_variant) {
        case H32:
          XXH32_freeState((XXH32_state_t*)_state);
          break;
        case H64:
          XXH64_freeState((XXH64_state_t*)_state);
          break;
        case H3:
        case H3_128:
          XXH3_freeState((XXH3_state_t*)_state);
          break;
      }
    }
  }

  XxHashDynamicState& operator=(XxHashDynamicState&& other) {
    _variant = other._variant;
    _state = other._state;
    other._state = nullptr;

    return *this;
  }

  void Reset(uint64_t seed) {
    switch (_variant) {
      case H32:
        XXH32_reset((XXH32_state_t*)_state, seed);
        break;
      case H64:
        XXH64_reset((XXH64_state_t*)_state, seed);
        break;
      case H3:
        XXH3_64bits_reset_withSeed((XXH3_state_t*)_state, seed);
        break;
      case H3_128:
        XXH3_128bits_reset_withSeed((XXH3_state_t*)_state, seed);
        break;
    }
  }

  void Update(const uint8_t* data, size_t length) {
    switch (_variant) {
      case H32:
        XXH32_update((XXH32_state_t*)_state, data, length);
        break;
      case H64:
        XXH64_update((XXH64_state_t*)_state, data, length);
        break;
      case H3:
        XXH3_64bits_update((XXH3_state_t*)_state, data, length);
        break;
      case H3_128:
        XXH3_128bits_update((XXH3_state_t*)_state, data, length);
        break;
    }
  }

  GenericHashResult GetResult() const {
    switch (_variant) {
      case H32:
        return XXH32_digest((XXH32_state_t*)_state);
      case H64:
        return XXH64_digest((XXH64_state_t*)_state);
      case H3:
        return XXH3_64bits_digest((XXH3_state_t*)_state);
      case H3_128:
        return XXH3_128bits_digest((XXH3_state_t*)_state);
      default:
        return GenericHashResult();
    }
  }

  static GenericHashResult Oneshot(uint32_t variant, const uint8_t* data,
                                   size_t length, uint64_t seed) {
    switch (variant) {
      case H32:
        return XXH32(data, length, seed);
      case H64:
        return XXH64(data, length, seed);
        break;
      case H3:
        return XXH3_64bits_withSeed(data, length, seed);
      case H3_128:
        return XXH3_128bits_withSeed(data, length, seed);
      default:
        return GenericHashResult();
    }
  }

 private:
  uint32_t _variant;
  void* _state = nullptr;
};
