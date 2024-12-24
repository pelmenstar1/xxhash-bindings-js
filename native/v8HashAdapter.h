#include <v8.h>
#include "hashers.h"

template<int Variant>
class V8HashAdapter {
public:
  static v8::Local<v8::Value> TransformResult(v8::Isolate* isolate, XxResult<Variant> result);
};