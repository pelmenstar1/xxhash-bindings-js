#include "v8HashState.h"

#include "helpers.h"
#include "v8ObjectParser.h"
#include "v8Utils.h"

template <int Variant>
Nan::Persistent<v8::Function> V8HashStateObject<Variant>::_constructor;

template <int Variant>
void V8HashStateObject<Variant>::Init() {
  auto context = Nan::GetCurrentContext();

  Nan::HandleScope scope;

  v8::Local<v8::FunctionTemplate> tpl = Nan::New<v8::FunctionTemplate>(New);
  tpl->SetClassName(Nan::New("XxHashState").ToLocalChecked());
  tpl->InstanceTemplate()->SetInternalFieldCount(2);

  Nan::SetPrototypeMethod(tpl, "update", Update);
  Nan::SetPrototypeMethod(tpl, "reset", Reset);
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

  try {
    XxSeed<Variant> seed = 0;
    switch (info.Length()) {
      case 1:
        V8_PARSE_ARGUMENT(seed, 0, XxSeed<Variant>, 0);
        break;
      default:
        throw std::runtime_error("Wrong number of arguments");
    }

    auto obj = new V8HashStateObject<Variant>(seed);
    obj->Wrap(info.This());

    info.GetReturnValue().Set(info.This());
  } catch (const std::exception& exc) {
    isolate->ThrowError(Nan::New(exc.what()).ToLocalChecked());
  }
}

template <int Variant>
void V8HashStateObject<Variant>::Reset(
    const Nan::FunctionCallbackInfo<v8::Value>& info) {
  auto obj = ObjectWrap::Unwrap<V8HashStateObject<Variant>>(info.Holder());
  auto&& state = obj->_state;

  state.Reset(obj->_seed);
}

template <int Variant>
void V8HashStateObject<Variant>::Update(
    const Nan::FunctionCallbackInfo<v8::Value>& info) {
  auto isolate = info.GetIsolate();

  try {
    auto obj = ObjectWrap::Unwrap<V8HashStateObject<Variant>>(info.Holder());
    auto&& state = obj->_state;

    RawSizedArray data;

    switch (info.Length()) {
      case 1:
        V8_PARSE_ARGUMENT(data, 0, RawSizedArray);
        break;
      default:
        throw std::runtime_error("Wrong number of arguments");
    }

    state.Update(data.data, data.length);
  } catch (const std::exception& exc) {
    isolate->ThrowError(Nan::New(exc.what()).ToLocalChecked());
  }
}

template <int Variant>
void V8HashStateObject<Variant>::GetResult(
    const Nan::FunctionCallbackInfo<v8::Value>& info) {
  auto isolate = info.GetIsolate();

  try {
    auto obj = ObjectWrap::Unwrap<V8HashStateObject<Variant>>(info.Holder());

    XxResult<Variant> result = obj->_state.GetResult();
    auto v8Result =
        V8ValueConverter<XxResult<Variant>>::ConvertBack(isolate, result);

    info.GetReturnValue().Set(v8Result);
  } catch (const std::exception& exc) {
    isolate->ThrowError(Nan::New(exc.what()).ToLocalChecked());
  }
}

template <int Variant>
v8::MaybeLocal<v8::Object> V8HashStateObject<Variant>::NewInstance(
    v8::Local<v8::Context> context, v8::Local<v8::Value> seed) {
  Nan::EscapableHandleScope scope;

  const unsigned argc = 1;
  v8::Local<v8::Value> argv[argc] = {seed};

  v8::Local<v8::Function> cons = Nan::New<v8::Function>(_constructor);

  auto instance = cons->NewInstance(context, argc, argv);

  v8::Local<v8::Object> instanceValue;
  if (instance.ToLocal(&instanceValue)) {
    return scope.Escape(instanceValue);
  }

  return {};
}

template class V8HashStateObject<H32>;
template class V8HashStateObject<H64>;
template class V8HashStateObject<H3>;
template class V8HashStateObject<H3_128>;