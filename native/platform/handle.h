#pragma once

#ifdef _WIN32
#include <windows.h>

using _FileHandleValue = HANDLE;
static const HANDLE _InvalidHandle = INVALID_HANDLE_VALUE;

#define _CLOSE_HANDLE CloseHandle
#define _NATIVE_CHAR WCHAR

#else
#include <unistd.h>
#include <fcntl.h>

using _FileHandleValue = int;
static const int _InvalidHandle = -1;

#define _CLOSE_HANDLE close
#define _NATIVE_CHAR char

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

  ~FileHandle() {
    if (fd != _InvalidHandle) {
      _CLOSE_HANDLE(fd);
    }
  }

  operator _FileHandleValue() {
    return fd;
  }

  bool IsInvalid() const { return fd == _InvalidHandle; }

  FileHandle& operator=(FileHandle&& other) {
    fd = other.fd;
    other.fd = _InvalidHandle;

    return *this;
  }

  static FileHandle OpenRead(const _NATIVE_CHAR* path) {
#ifdef _WIN32
    HANDLE fd = CreateFileW(path, GENERIC_READ, FILE_SHARE_READ, NULL,
                            OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
#else
    int fd = open(path, O_RDONLY);
#endif

    return {fd};
  }
};