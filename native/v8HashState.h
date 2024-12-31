#include <nan.h>

#include <cstdint>

#include "hashers.h"

template <int Variant>
class V8HashStateObject : public Nan::ObjectWrap {
 public:
  static v8::MaybeLocal<v8::Object> NewInstance(v8::Local<v8::Context> context, v8::Local<v8::Value> seed);

  static void Init();

 private:
  static Nan::Persistent<v8::Function> _constructor;

  V8HashStateObject(XxSeed<Variant> seed) : _state({seed}), _seed(seed) {}

  static void New(const Nan::FunctionCallbackInfo<v8::Value>& info);
  static void Reset(const Nan::FunctionCallbackInfo<v8::Value>& info);
  static void Update(const Nan::FunctionCallbackInfo<v8::Value>& info);
  static void GetResult(const Nan::FunctionCallbackInfo<v8::Value>& info);

  XxHashState<Variant> _state;
  XxSeed<Variant> _seed;
};

class V8HashStateObjectManager {
 public:
  static void Init();
};