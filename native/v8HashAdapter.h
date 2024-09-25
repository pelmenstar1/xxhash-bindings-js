#include <v8.h>
#include "hashers.h"

template<int Variant>
class V8HashAdapter {
public:
  static std::optional<XxSeed<Variant>> GetSeed(v8::Isolate* isolate, v8::Local<v8::Value> seed);
  static v8::Local<v8::Value> TransformResult(v8::Isolate* isolate, XxResult<Variant> result);
};