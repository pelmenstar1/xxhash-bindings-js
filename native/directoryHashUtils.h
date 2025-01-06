#include <v8.h>

enum FileGateResult { True = 1, False = 0, Error = -1 };

FileGateResult CallAcceptFile(v8::Isolate* isolate,
                              v8::Local<v8::Function> acceptFile,
                              v8::Local<v8::String> fileName);

FileGateResult ConvertToFileGateResult(v8::Isolate* isolate,
                                       v8::MaybeLocal<v8::Value> value);