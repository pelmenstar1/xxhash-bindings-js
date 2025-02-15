#pragma once

#include "nativeString.h"

#ifdef _WIN32
#include <windows.h>

using _FileHandleValue = HANDLE;
static const HANDLE _InvalidHandle = INVALID_HANDLE_VALUE;

#define _CLOSE_HANDLE CloseHandle

#else
#include <fcntl.h>
#include <unistd.h>

using _FileHandleValue = int;
static const int _InvalidHandle = -1;

#define _CLOSE_HANDLE close

#endif

class FileHandle {
 public:
  _FileHandleValue fd;

  FileHandle() : fd(_InvalidHandle) {}
  FileHandle(const FileHandle& other) = delete;
  FileHandle(FileHandle&& other) {
    fd = other.fd;
    other.fd = _InvalidHandle;
  }
  FileHandle(_FileHandleValue fd) : fd(fd) {}

  ~FileHandle() { Close(); }

  operator _FileHandleValue() { return fd; }

  bool IsInvalid() const { return fd == _InvalidHandle; }

  FileHandle& operator=(FileHandle&& other) {
    if (fd != other.fd) {
      Close();
    }
    
    fd = other.fd;
    other.fd = _InvalidHandle;

    return *this;
  }

  void Close() {
    if (fd != _InvalidHandle) {
      _CLOSE_HANDLE(fd);
    }
  }

  static FileHandle OpenRead(const NativeString& path) {
#ifdef _WIN32
    HANDLE fd = CreateFileW((LPCWSTR)path.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL,
                            OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
#else
    int fd = open(path.c_str(), O_RDONLY);
#endif

    return {fd};
  }
};