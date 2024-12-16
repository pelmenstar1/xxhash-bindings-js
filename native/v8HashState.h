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

  V8HashStateObject(XxHashState<Variant> state) : _state(std::move(state)) {}

  static void New(const Nan::FunctionCallbackInfo<v8::Value>& info);
  static void Update(const Nan::FunctionCallbackInfo<v8::Value>& info);
  static void GetResult(const Nan::FunctionCallbackInfo<v8::Value>& info);

  XxHashState<Variant> _state;
};

class V8HashStateObjectManager {
 public:
  static void Init();
};