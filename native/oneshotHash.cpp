#include "exports.h"
#include "hashers.h"
#include "helpers.h"
#include "v8ObjectParser.h"

template <int Variant>
void OneshotHash(const Nan::FunctionCallbackInfo<v8::Value>& info) {
  try {
    v8::Isolate* isolate = info.GetIsolate();

    RawSizedArray data;
    XxSeed<Variant> seed = 0;

    switch (info.Length()) {
      case 2: 
        V8_PARSE_ARGUMENT(seed, 1, XxSeed<Variant>, 0);
      case 1:
        V8_PARSE_ARGUMENT(data, 0, RawSizedArray);
        break;
      default:
        throw std::runtime_error("Wrong number of arguments");
    }

    auto result = XxHasher<Variant>::Process(data.data, data.length, seed);

    info.GetReturnValue().Set(
        V8ValueConverter<XxResult<Variant>>::ConvertBack(isolate, result));
  } catch (std::exception& exc) {
    Nan::ThrowError(exc.what());
  }
}

INSTANTIATE_HASH_FUNCTION(OneshotHash)