#include <nan.h>

#include <cstdint>

#include "hashers.h"

class V8HashStateObjectStaticData {
 public:
  V8HashStateObjectStaticData(v8::Local<v8::Context> context);
 
  const Nan::Persistent<v8::Function>& Get(uint32_t index) const {
    return _constructors[index];
  }
 private:
  Nan::Persistent<v8::Function> _constructors[4];

  template<int Variant>
  void InitConstructor(v8::Local<v8::Context> context);

  static void DeleteInstance(void* data) {
    delete static_cast<V8HashStateObjectStaticData*>(data);
  }
};

template <int Variant>
class V8HashStateObject : public Nan::ObjectWrap {
 public:
  static v8::MaybeLocal<v8::Object> NewInstance(
      v8::Local<v8::Context> context,
      const V8HashStateObjectStaticData& staticData, v8::Local<v8::Value> seed);

  static void New(const Nan::FunctionCallbackInfo<v8::Value>& info);
  static void Reset(const Nan::FunctionCallbackInfo<v8::Value>& info);
  static void Update(const Nan::FunctionCallbackInfo<v8::Value>& info);
  static void GetResult(const Nan::FunctionCallbackInfo<v8::Value>& info);

 private:
  V8HashStateObject(XxSeed<Variant> seed) : _state({seed}), _seed(seed) {}

  XxHashState<Variant> _state;
  XxSeed<Variant> _seed;
};
