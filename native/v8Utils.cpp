#include "v8Utils.h"

#include <nan.h>

#include <stdexcept>

NativeString V8StringToNative(v8::Isolate* isolate,
                              v8::Local<v8::String> text) {
#ifdef _WIN32
  int length = text->Length();
  auto buffer = new wchar_t[length + 1];
  text->Write(isolate, (uint16_t*)buffer, 0, length);
#else
  int length = text->Utf8Length(isolate);
  auto buffer = new char[length + 1];
  text->WriteUtf8(isolate, buffer);
#endif

  buffer[length] = 0;

  return NativeString(buffer, length);
}

RawSizedArray V8GetBackingStorage(v8::Local<v8::Uint8Array> array) {
  uint8_t* data = (uint8_t*)array->Buffer()->Data();
  size_t offset = array->ByteOffset();
  size_t length = array->ByteLength();

  return {data + offset, length};
}

template <typename T>
std::optional<T> MaybeToOptional(v8::Maybe<T> input) {
  return input.IsNothing() ? std::optional<T>() : input.ToChecked();
}

std::optional<uint32_t> V8GetUInt32Optional(v8::Isolate* isolate,
                                            v8::Local<v8::Value> value,
                                            uint32_t ifUndefined) {
  if (value->IsNumber()) {
    return MaybeToOptional(value->Uint32Value(isolate->GetCurrentContext()));
  }

  if (value->IsNullOrUndefined()) {
    return ifUndefined;
  }

  return {};
}

std::optional<uint64_t> V8GetUInt64Optional(v8::Isolate* isolate,
                                            v8::Local<v8::Value> value,
                                            uint64_t ifUndefined) {
  if (value->IsNumber()) {
    return MaybeToOptional(value->IntegerValue(isolate->GetCurrentContext()));
  }

  if (value->IsBigInt()) {
    auto bigint = value->ToBigInt(isolate->GetCurrentContext());

    return bigint.IsEmpty() ? std::optional<uint64_t>()
                            : bigint.ToLocalChecked()->Uint64Value();
  }

  if (value->IsNullOrUndefined()) {
    return ifUndefined;
  }

  return {};
}

v8::Local<v8::Value> V8CreateUInt128Number(v8::Isolate* isolate, uint64_t low,
                                           uint64_t high) {
  uint64_t words[2] = {low, high};

  return v8::BigInt::NewFromWords(isolate->GetCurrentContext(), 0, 2, words)
      .ToLocalChecked();
}

v8::MaybeLocal<v8::Value> V8GetObjectProperty(v8::Local<v8::Context> context,
                                              v8::Local<v8::Object> obj,
                                              const char* name) {
  auto key = Nan::New(name);
  if (key.IsEmpty()) {
    throw std::runtime_error("Cannot create a string");
  }

  return obj->Get(context, key.ToLocalChecked());
}
