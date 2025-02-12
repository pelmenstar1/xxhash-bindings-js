#include <nan.h>

#define DECLARE_HASH_FUNCTION(name) \
  template <int Variant>            \
  void name(const Nan::FunctionCallbackInfo<v8::Value>& info);

DECLARE_HASH_FUNCTION(OneshotHash)
DECLARE_HASH_FUNCTION(CreateHashState)
DECLARE_HASH_FUNCTION(FileHash)
DECLARE_HASH_FUNCTION(FileHashAsync)
DECLARE_HASH_FUNCTION(DirectoryHash)
DECLARE_HASH_FUNCTION(DirectoryHashAsync)
DECLARE_HASH_FUNCTION(DirectoryToMapHash)
DECLARE_HASH_FUNCTION(DirectoryToMapHashAsync)
