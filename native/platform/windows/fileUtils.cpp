#ifdef _WIN32

#include "../fileUtils.h"

HANDLE OpenFileWithV8Path(v8::Isolate* isolate,
                          v8::Local<v8::String> pathValue) {
  int length = pathValue->Length();

  auto path = std::make_unique<uint16_t[]>(length + 1);
  pathValue->Write(isolate, path.get(), 0, length);
  path[length] = 0;

  return CreateFileW((LPCWSTR)path.get(), GENERIC_READ, FILE_SHARE_READ, NULL,
                     OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
}

#endif