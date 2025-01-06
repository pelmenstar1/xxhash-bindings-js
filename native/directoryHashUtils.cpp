#include "directoryHashUtils.h"

#include "v8Utils.h"

FileGateResult CallAcceptFile(v8::Isolate* isolate,
                              v8::Local<v8::Function> acceptFile,
                              v8::Local<v8::String> fileName) {
  if (acceptFile.IsEmpty()) {
    return FileGateResult::True;
  }

  auto maybeResult = V8CallFunction(isolate, acceptFile, fileName);
  return ConvertToFileGateResult(isolate, maybeResult);
}

FileGateResult ConvertToFileGateResult(v8::Isolate* isolate,
                                       v8::MaybeLocal<v8::Value> value) {
  v8::Local<v8::Value> result;
  if (value.ToLocal(&result)) {
    // Use more general way to convert the returned value to boolean
    // in order to be more aligned with the "minimum" implementation which
    // just does if (acceptGate ...)
    return (FileGateResult)result->ToBoolean(isolate)->Value();
  }

  return FileGateResult::Error;
}
