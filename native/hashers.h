#pragma once

#include <cstdint>
#include <stdexcept>

#include "xxhash.h"

enum HashVariant { H32, H64, H3, H3_128 };

constexpr uint32_t HASH_VARIANTS_COUNT = 4;

using GenericHashResult = XXH128_hash_t;

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

  XXH128_hash_t GetResult() const {
    switch (_variant) {
      case H32:
        return {.low64 = XXH32_digest((XXH32_state_t*)_state), .high64 = 0};
      case H64:
        return {.low64 = XXH64_digest((XXH64_state_t*)_state), .high64 = 0};
      case H3:
        return {.low64 = XXH3_64bits_digest((XXH3_state_t*)_state),
                .high64 = 0};
      case H3_128:
        return XXH3_128bits_digest((XXH3_state_t*)_state);
      default:
        return {.low64 = 0, .high64 = 0};
    }
  }

  static GenericHashResult Oneshot(uint32_t variant, const uint8_t* data,
                                   size_t length, uint64_t seed) {
    switch (variant) {
      case H32:
        return {.low64 = XXH32(data, length, seed), .high64=0};
      case H64:
        return {.low64 = XXH64(data, length, seed), .high64=0};
        break;
      case H3:
        return {.low64 = XXH3_64bits_withSeed(data, length, seed), .high64=0};
      case H3_128:
        return XXH3_128bits_withSeed(data, length, seed);
      default:
        return {.low64 = 0, .high64 = 0};
    }
  }

 private:
  uint32_t _variant;
  void* _state = nullptr;
};
