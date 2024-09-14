#define THROW_INVALID_ARG_COUNT                     \
  Nan::ThrowTypeError("Wrong number of arguments"); \
  return

#define THROW_INVALID_ARG_TYPE(argNum, expectedType)           \
  Nan::ThrowTypeError("Expected type of the argument " #argNum \
                      " is " expectedType);                    \
  return

#define CHECK_SEED(seedArg)                               \
  if (!(seedArg)->IsNumber() && !(seedArg)->IsBigInt()) { \
    THROW_INVALID_ARG_TYPE(2, "number or bigint");        \
  }

#define CHECK_SEED_UNDEFINED(seedArg)                         \
  if (!(seedArg)->IsNumber() && !(seedArg)->IsBigInt() &&     \
      !(seedArg->IsNullOrUndefined())) {                      \
    THROW_INVALID_ARG_TYPE(2, "number, bigint or undefined"); \
  }

#define CHECK_PLATFORM_ERROR(variable, value) \
  if ((variable) == (value)) {                \
    return PlatformOperationStatus::Error();  \
  }
