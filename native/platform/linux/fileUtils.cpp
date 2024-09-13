#include "../fileUtils.h"

#include <unistd.h>
#include <fcntl.h>

FileDescriptor OpenFileWithV8Path(v8::Isolate* isolate,
                                  v8::Local<v8::String> pathValue) {
  int length = pathValue->Utf8Length(isolate);

  auto path = std::make_unique<char[]>(length + 1);
  pathValue->WriteUtf8(isolate, path.get());
  path[length] = 0;

  return open(path.get(), O_RDONLY);                                    
}