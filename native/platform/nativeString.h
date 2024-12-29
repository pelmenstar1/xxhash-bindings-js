#pragma once

#include <string>

#ifdef _WIN32
using NativeChar = wchar_t;

#define NativeStringLen wcslen
#else
using NativeChar = char;

#define NativeStringLen strlen
#endif

using NativeString = std::basic_string<NativeChar>;
using NativeStringView = std::basic_string_view<NativeChar>;
