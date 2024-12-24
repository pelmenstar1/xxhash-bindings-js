#include <nan.h>

#include <functional>

#include "exports.h"
#include "helpers.h"
#include "v8HashState.h"

struct ExportedFunctionToken {
  const char* name;
  Nan::FunctionCallback function;
};

#define FUNCTION_SET_ITEM(name, func, variant) {name, func##__exc<variant>}

#define FUNCTION_SET(suffix, function)                       \
  FUNCTION_SET_ITEM("xxhash32_" #suffix, function, H32),     \
      FUNCTION_SET_ITEM("xxhash64_" #suffix, function, H64), \
      FUNCTION_SET_ITEM("xxhash3_" #suffix, function, H3),   \
      FUNCTION_SET_ITEM("xxhash3_128_" #suffix, function, H3_128)

WITH_EXCEPTION_HANDLING(OneshotHash)
WITH_EXCEPTION_HANDLING(CreateHashState)

#ifndef XXHASH_BINDINGS_MIN
WITH_EXCEPTION_HANDLING(FileHash)
#endif

void Init(v8::Local<v8::Object> exports) {
  v8::Local<v8::Context> context =
      exports->GetCreationContext().ToLocalChecked();

  V8HashStateObjectManager::Init();

  ExportedFunctionToken exportedFunctions[] = {
      FUNCTION_SET(oneshot, OneshotHash),
      FUNCTION_SET(createState, CreateHashState),

#ifndef XXHASH_BINDINGS_MIN
      FUNCTION_SET(file, FileHash),
#endif
  };

  for (auto& token : exportedFunctions) {
    exports
        ->Set(context, Nan::New(token.name).ToLocalChecked(),
              Nan::New<v8::FunctionTemplate>(token.function)
                  ->GetFunction(context)
                  .ToLocalChecked())
        .Check();
  }
}

NODE_MODULE(xxhash, Init)