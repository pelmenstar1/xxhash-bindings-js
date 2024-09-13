#include <nan.h>

#include <cstdint>
#include <optional>

#include "xxhash.h"

enum HashVariant { H32, H64, H3, H3_128 };

template <int Variant>
struct XxHashInternalStateHolder {};

#define INTERNAL_STATE_SPEC(type, stateType) \
  template <>                                \
  struct XxHashInternalStateHolder<type> {   \
    stateType* state = nullptr;              \
  };

INTERNAL_STATE_SPEC(H32, XXH32_state_t)
INTERNAL_STATE_SPEC(H64, XXH64_state_t)
INTERNAL_STATE_SPEC(H3, XXH3_state_t)
INTERNAL_STATE_SPEC(H3_128, XXH3_state_t)

#undef INTERNAL_STATE_SPEC

typedef std::optional<v8::Local<v8::Value>> V8OptionalSeed;

template <int Variant>
class XxHashState {
 public:
  XxHashState(v8::Isolate* isolate) : _isolate(isolate) {}
  ~XxHashState();

  bool Init(V8OptionalSeed seed);
  void Update(const uint8_t* data, size_t length);

  v8::Local<v8::Value> GetResult();

 private:
  v8::Isolate* _isolate;
  XxHashInternalStateHolder<Variant> _stateHolder;
};

template <int Variant>
struct XxHasher {
  static XxHashState<Variant> CreateState(v8::Isolate* isolate) {
    return XxHashState<Variant>(isolate);
  }

  static v8::Local<v8::Value> Process(v8::Isolate* isolate, const uint8_t* data,
                                      size_t length, V8OptionalSeed seed);
};