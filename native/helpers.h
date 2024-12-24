#define _INSTANTIATE_HASH_FUNCTION(name, variant) \
  template void name<variant>(const Nan::FunctionCallbackInfo<v8::Value>& info);

#define INSTANTIATE_HASH_FUNCTION(name) \
  _INSTANTIATE_HASH_FUNCTION(name, H32) \
  _INSTANTIATE_HASH_FUNCTION(name, H64) \
  _INSTANTIATE_HASH_FUNCTION(name, H3)  \
  _INSTANTIATE_HASH_FUNCTION(name, H3_128)

#define WITH_EXCEPTION_HANDLING(function)                                      \
  template <int Variant>                                                   \
  static void function##__exc(const Nan::FunctionCallbackInfo<v8::Value>& info) { \
    try {                                                                  \
      function<Variant>(info);                                             \
    } catch (std::exception & exc) {                                       \
      Nan::ThrowError(exc.what());                                         \
    }                                                                      \
  }
