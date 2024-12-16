#include "v8HashState.h"

#include "errorMacro.h"
#include "helpers.h"
#include "v8HashAdapter.h"
#include "v8Utils.h"

template <int Variant>
Nan::Persistent<v8::Function> V8HashStateObject<Variant>::_constructor;

template <int Variant>
void V8HashStateObject<Variant>::Init() {
  auto context = Nan::GetCurrentContext();

  Nan::HandleScope scope;

  v8::Local<v8::FunctionTemplate> tpl = Nan::New<v8::FunctionTemplate>(New);
  tpl->SetClassName(Nan::New("XxHashState").ToLocalChecked());
  tpl->InstanceTemplate()->SetInternalFieldCount(1);

  Nan::SetPrototypeMethod(tpl, "update", Update);
  Nan::SetPrototypeMethod(tpl, "result", GetResult);

  _constructor.Reset(tpl->GetFunction(context).ToLocalChecked());
}

void V8HashStateObjectManager::Init() {
  V8HashStateObject<H32>::Init();
  V8HashStateObject<H64>::Init();
  V8HashStateObject<H3>::Init();
  V8HashStateObject<H3_128>::Init();
}

template <int Variant>
void V8HashStateObject<Variant>::New(
    const Nan::FunctionCallbackInfo<v8::Value>& info) {
  auto isolate = info.GetIsolate();
  auto context = isolate->GetCurrentContext();

  int argCount = info.Length();
  if (argCount != 1) {
    THROW_INVALID_ARG_COUNT;
  }

  XxSeed<Variant> seed = 0;

  if (argCount > 0) {
    auto optSeed = V8HashAdapter<Variant>::GetSeed(isolate, info[0]);
    if (!optSeed) {
      THROW_INVALID_ARG_TYPE(1, "number or bigint");
    }

    seed = optSeed.value();
  }

  XxHashState<Variant> state;
  if (!state.Init(seed)) {
    Nan::ThrowError("Cannot initialize a hash state");
    return;
  }

  auto obj = new V8HashStateObject<Variant>(std::move(state));
  obj->Wrap(info.This());

  info.GetReturnValue().Set(info.This());
}

template <int Variant>
void V8HashStateObject<Variant>::Update(
    const Nan::FunctionCallbackInfo<v8::Value>& info) {
  auto obj = ObjectWrap::Unwrap<V8HashStateObject<Variant>>(info.Holder());
  auto&& state = obj->_state;

  if (info.Length() != 1) {
    THROW_INVALID_ARG_COUNT;
  }

  auto bufferArg = info[0];

  if (!bufferArg->IsUint8Array()) {
    THROW_INVALID_ARG_TYPE(1, "Uint8Array");
  }

  auto buffer = bufferArg.As<v8::Uint8Array>()->Buffer();
  uint8_t* data = (uint8_t*)buffer->Data();
  size_t length = buffer->ByteLength();

  state.Update(data, length);
}

template <int Variant>
void V8HashStateObject<Variant>::GetResult(
    const Nan::FunctionCallbackInfo<v8::Value>& info) {
  auto isolate = info.GetIsolate();
  auto obj = ObjectWrap::Unwrap<V8HashStateObject<Variant>>(info.Holder());

  XxResult<Variant> result = obj->_state.GetResult();
  auto v8Result = V8HashAdapter<Variant>::TransformResult(isolate, result);

  info.GetReturnValue().Set(v8Result);
}

template <int Variant>
v8::Local<v8::Object> V8HashStateObject<Variant>::NewInstance(
    v8::Local<v8::Value> seed) {
  Nan::EscapableHandleScope scope;

  const unsigned argc = 1;
  v8::Local<v8::Value> argv[argc] = {seed};

  v8::Local<v8::Function> cons = Nan::New<v8::Function>(_constructor);
  v8::Local<v8::Context> context =
      v8::Isolate::GetCurrent()->GetCurrentContext();
  v8::Local<v8::Object> instance =
      cons->NewInstance(context, argc, argv).ToLocalChecked();

  return scope.Escape(instance);
}

template class V8HashStateObject<H32>;
template class V8HashStateObject<H64>;
template class V8HashStateObject<H3>;
template class V8HashStateObject<H3_128>;