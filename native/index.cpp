#include <nan.h>
#include "exports.h"
#include <functional>
#include "v8HashState.h"

struct ExportedFunctionToken {
  const char* name;
  Nan::FunctionCallback function;
};

#define CONCAT(a, b) a # b

#define FUNCTION_SET(suffix, function) \
  { "xxhash32_" # suffix, function<H32> }, \
  { "xxhash64_" # suffix, function<H64> }, \
  { "xxhash3_" # suffix, function<H3> }, \
  { "xxhash3_128_" # suffix, function<H3_128> } \

void Init(v8::Local<v8::Object> exports) {
  v8::Local<v8::Context> context =
      exports->GetCreationContext().ToLocalChecked();

  V8HashStateObjectManager::Init();

  ExportedFunctionToken exportedFunctions[] = {
    FUNCTION_SET(oneshot, OneshotHash),
    FUNCTION_SET(file, FileHash),
    FUNCTION_SET(createState, CreateHashState),
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