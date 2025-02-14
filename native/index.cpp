#include <nan.h>

#include <functional>

#include "exports.h"
#include "helpers.h"
#include "v8HashState.h"

struct ExportedFunctionToken {
  const char* name;
  Nan::FunctionCallback function;
};

#define FUNCTION_SET(suffix, function)                                        \
  {"xxhash32_" #suffix, function<H32>}, {"xxhash64_" #suffix, function<H64>}, \
      {"xxhash3_" #suffix, function<H3>}, {                                   \
    "xxhash3_128_" #suffix, function<H3_128>                                  \
  }

void Init(v8::Local<v8::Object> exports) {}

#define NODE_GYP_MODULE_NAME xxhash

NODE_MODULE_INIT(/* exports, module, context */) {
  auto isolate = context->GetIsolate();
  auto staticData = new V8HashStateObjectStaticData(context);
  auto external = v8::External::New(isolate, staticData);

  ExportedFunctionToken exportedFunctions[] = {
      FUNCTION_SET(oneshot, OneshotHash),
      FUNCTION_SET(createState, CreateHashState),

      FUNCTION_SET(file, FileHash),
      FUNCTION_SET(directory, DirectoryHash),
      FUNCTION_SET(directoryToMap, DirectoryToMapHash),

      FUNCTION_SET(fileAsync, FileHashAsync),
      FUNCTION_SET(directoryAsync, DirectoryHashAsync),
      FUNCTION_SET(directoryToMapAsync, DirectoryToMapHashAsync)};

  for (auto& token : exportedFunctions) {
    exports
        ->Set(context, Nan::New(token.name).ToLocalChecked(),
              Nan::New<v8::FunctionTemplate>(token.function, external)
                  ->GetFunction(context)
                  .ToLocalChecked())
        .Check();
  }
}
