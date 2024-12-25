#pragma once

#include <string>

#ifdef _WIN32
using NativeChar = wchar_t;
#else
using NativeChar = char;
#endif

using NativeString = std::basic_string<NativeChar>;
