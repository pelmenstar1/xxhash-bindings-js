#include "v8HashAdapter.h"

#include <cstdint>

#include "v8Utils.h"

static v8::Local<v8::Value> XxHash128ResultToBigInt(v8::Isolate* isolate,
                                                    XXH128_hash_t value) {
  return V8CreateUInt128Number(isolate, value.low64, value.high64);
}

#define IMPL_GET_SEED(variant, func)                              \
  template<> \
  std::optional<XxSeed<variant>> V8HashAdapter<variant>::GetSeed( \
      v8::Isolate* isolate, v8::Local<v8::Value> seed) {          \
    return func(isolate, seed);                                   \
  }

#define IMPL_TRANSFORM_RESULT(variant, func)                    \
  template<> \
  v8::Local<v8::Value> V8HashAdapter<variant>::TransformResult( \
      v8::Isolate* isolate, XxResult<variant> result) {         \
    return func(isolate, result);                               \
  }

IMPL_GET_SEED(H32, V8GetUInt32Optional)
IMPL_GET_SEED(H64, V8GetUInt64Optional)
IMPL_GET_SEED(H3, V8GetUInt64Optional)
IMPL_GET_SEED(H3_128, V8GetUInt64Optional)

IMPL_TRANSFORM_RESULT(H32, v8::Integer::NewFromUnsigned)
IMPL_TRANSFORM_RESULT(H64, v8::BigInt::NewFromUnsigned)
IMPL_TRANSFORM_RESULT(H3, v8::BigInt::NewFromUnsigned)
IMPL_TRANSFORM_RESULT(H3_128, XxHash128ResultToBigInt)