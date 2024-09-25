#include "v8Utils.h"

#include "errorMacro.h"

std::unique_ptr<char[]> V8StringToUtf8(v8::Isolate* isolate,
                                       v8::Local<v8::String> text) {
  int length = text->Utf8Length(isolate);

  auto buffer = std::make_unique<char[]>(length + 1);
  text->WriteUtf8(isolate, buffer.get());
  buffer[length] = 0;

  return buffer;
}

std::unique_ptr<uint16_t[]> V8StringToUtf16(v8::Isolate* isolate,
                                            v8::Local<v8::String> text) {
  int length = text->Length();

  auto buffer = std::make_unique<uint16_t[]>(length + 1);
  text->Write(isolate, buffer.get(), 0, length);
  buffer[length] = 0;

  return buffer;
}

std::optional<uint32_t> V8GetUInt32Optional(v8::Isolate* isolate,
                                            v8::Local<v8::Value> value) {
  if (value->IsNumber()) {
    return value->Uint32Value(isolate->GetCurrentContext()).ToChecked();
  }

  if (value->IsNullOrUndefined()) {
    return 0;
  }

  return {};
}

std::optional<uint64_t> V8GetUInt64Optional(v8::Isolate* isolate,
                                    v8::Local<v8::Value> value) {
  if (value->IsNumber()) {
    return value->IntegerValue(isolate->GetCurrentContext()).ToChecked();
  }

  if (value->IsBigInt()) {
    return value->ToBigInt(isolate->GetCurrentContext())
        .ToLocalChecked()
        ->Uint64Value();
  }

  if (value->IsNullOrUndefined()) {
    return 0;
  }

  return {};
}

v8::Local<v8::Value> V8CreateUInt128Number(v8::Isolate* isolate, uint64_t low,
                                           uint64_t high) {
  uint64_t words[2] = {low, high};

  return v8::BigInt::NewFromWords(isolate->GetCurrentContext(), 0, 2, words)
      .ToLocalChecked();
}