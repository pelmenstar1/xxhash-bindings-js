#include "hashers.h"

#include <iostream>

#include "errorMacro.h"
#include "xxhash.h"

#define IMPLEMENT_STATE(type, create, update, reset, digest, free)     \
  template <>                                                          \
  XxHashState<type>::~XxHashState() {                                  \
    if (_state != nullptr) free(_state);                               \
  }                                                                    \
  template <>                                                          \
  bool XxHashState<type>::Init(XxSeed<type> seed) {                    \
    auto state = create();                                             \
    if (state == nullptr) return false;                                \
                                                                       \
    reset(state, seed);                                                \
    _state = state;                                                    \
    return true;                                                       \
  }                                                                    \
  template <>                                                          \
  void XxHashState<type>::Update(const uint8_t* data, size_t length) { \
    update(_state, data, length);                                      \
  }                                                                    \
  template <>                                                          \
  XxResult<type> XxHashState<type>::GetResult() {                      \
    return digest(_state);                                             \
  }

#define X32F(w, name) XXH##w##_##name
#define X3F(w, name) XXH3_##w##bits_##name

#define IMPLEMENT_32_64_STATE(w)                                               \
  IMPLEMENT_STATE(H##w, X32F(w, createState), X32F(w, update), X32F(w, reset), \
                  X32F(w, digest), X32F(w, freeState))

#define IMPLEMENT_STATE3(name, w)                         \
  IMPLEMENT_STATE(name, XXH3_createState, X3F(w, update), \
                  X3F(w, reset_withSeed), X3F(w, digest), XXH3_freeState)

IMPLEMENT_32_64_STATE(32)
IMPLEMENT_32_64_STATE(64)
IMPLEMENT_STATE3(H3, 64)
IMPLEMENT_STATE3(H3_128, 128)

#define PROCESS_FUNCTION(type)                                               \
  template <>                                                                \
  XxResult<type> XxHasher<type>::Process(v8::Isolate* isolate,               \
                                         const uint8_t* data, size_t length, \
                                         XxSeed<type> seed)

#define IMPLEMENT_PROCESS_32_64(width) \
  PROCESS_FUNCTION(H##width) { return XXH##width(data, length, seed); }

#define IMPLEMENT_PROCESS_3(type, width)                                \
  PROCESS_FUNCTION(type) {                                              \
    return seed == 0 ? XXH3_##width##bits(data, length)                 \
                     : XXH3_##width##bits_withSeed(data, length, seed); \
  }

IMPLEMENT_PROCESS_32_64(32)
IMPLEMENT_PROCESS_32_64(64)
IMPLEMENT_PROCESS_3(H3, 64)
IMPLEMENT_PROCESS_3(H3_128, 128)