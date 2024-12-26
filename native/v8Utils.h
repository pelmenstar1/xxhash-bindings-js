#pragma once

#include <v8.h>

#include <cstdint>
#include <memory>
#include <optional>

#include "platform/nativeString.h"

NativeString V8StringToNative(v8::Isolate* isolate, v8::Local<v8::String> text);

v8::MaybeLocal<v8::Value> V8GetPropertyValue(v8::Isolate* isolate,
                                           v8::Local<v8::Object> object,
                                           const char* name);
            