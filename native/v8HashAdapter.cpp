#include "v8HashAdapter.h"

#include <cstdint>

#include "v8Utils.h"

static v8::Local<v8::Value> XxHash128ResultToBigInt(v8::Isolate* isolate,
                                                    XXH128_hash_t value) {
  return V8CreateUInt128Number(isolate, value.low64, value.high64);
}

#define IMPL_TRANSFORM_RESULT(variant, func)                    \
  template<> \
  v8::Local<v8::Value> V8HashAdapter<variant>::TransformResult( \
      v8::Isolate* isolate, XxResult<variant> result) {         \
    return func(isolate, result);                               \
  }

IMPL_TRANSFORM_RESULT(H32, v8::Integer::NewFromUnsigned)
IMPL_TRANSFORM_RESULT(H64, v8::BigInt::NewFromUnsigned)
IMPL_TRANSFORM_RESULT(H3, v8::BigInt::NewFromUnsigned)
IMPL_TRANSFORM_RESULT(H3_128, XxHash128ResultToBigInt)