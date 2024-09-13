#ifndef _FILE_UTILS
#define _FILE_UTILS

#include <v8.h>

#ifdef _WIN32
#include <windows.h>

typedef HANDLE FileDescriptor;
#elif defined(unix)

typedef int FileDescriptor;
#endif

FileDescriptor OpenFileWithV8Path(v8::Isolate* isolate,
                                  v8::Local<v8::String> pathValue);

#endif