#define THROW_INVALID_ARG_COUNT                     \
  Nan::ThrowTypeError("Wrong number of arguments"); \
  return

#define THROW_INVALID_ARG_TYPE(argNum, expectedType)           \
  Nan::ThrowTypeError("Expected type of the argument " #argNum \
                      " is " expectedType);                    \
  return

#define _INSTANTIATE_HASH_FUNCTION(name, variant) \
  template void name<variant>(const Nan::FunctionCallbackInfo<v8::Value>& info);

#define INSTANTIATE_HASH_FUNCTION(name) \
  _INSTANTIATE_HASH_FUNCTION(name, H32) \
  _INSTANTIATE_HASH_FUNCTION(name, H64) \
  _INSTANTIATE_HASH_FUNCTION(name, H3)  \
  _INSTANTIATE_HASH_FUNCTION(name, H3_128)
