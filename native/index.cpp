#include <nan.h>
#include "exports.h"
#include <functional>
#include "v8HashState.h"

struct ExportedFunctionToken {
  const char* name;
  Nan::FunctionCallback function;
};

void Init(v8::Local<v8::Object> exports) {
  v8::Local<v8::Context> context =
      exports->GetCreationContext().ToLocalChecked();

  V8HashStateObjectManager::Init();

  ExportedFunctionToken exportedFunctions[] = {
    { "xxhash32", XxHash32 },
    { "xxhash64", XxHash64 },
    { "xxhash3", XxHash3 },
    { "xxhash3_128", XxHash3_128 },
    { "xxhash32_file", XxHash32File },
    { "xxhash64_file", XxHash64File },
    { "xxhash3_file", XxHash3File },
    { "xxhash3_128_file", XxHash3_128_File },
    { "xxhash32_createState", CreateXxHash32State },
    { "xxhash64_createState", CreateXxHash64State },
    { "xxhash3_createState", CreateXxHash3State },
    { "xxhash3_128_createState", CreateXxHash3_128_State },
  };

  for (auto& token : exportedFunctions) {
    exports->Set(
      context, 
      Nan::New(token.name).ToLocalChecked(),
      Nan::New<v8::FunctionTemplate>(token.function)
          ->GetFunction(context)
          .ToLocalChecked()
    ).Check();
  }
}

NODE_MODULE(xxhash, Init)