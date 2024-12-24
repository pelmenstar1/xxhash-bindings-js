#ifdef _WIN32
#include <windows.h>

using _FileHandleValue = HANDLE;
static const HANDLE _InvalidHandle = INVALID_HANDLE_VALUE;

#define _CLOSE_HANDLE CloseHandle

#else
#include <unistd.h>

using _FileHandleValue = int;
static const int _InvalidHandle = -1;

#define _CLOSE_HANDLE close

#endif

class FileHandle {
 public:
  _FileHandleValue fd;

  FileHandle(): fd(_InvalidHandle) {}
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
};