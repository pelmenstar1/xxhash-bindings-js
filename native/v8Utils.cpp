#include "v8Utils.h"

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
