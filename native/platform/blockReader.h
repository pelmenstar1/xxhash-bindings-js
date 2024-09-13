#include <cstdint>

#include "platformOperationStatus.h"

#ifdef _WIN32
#include <windows.h>
#endif

struct Block {
  uint8_t* data;
  uint32_t length;
};

class ReadBlockResult {
 private:
  bool _isSuccess;
  PlatformError _error;
  uint8_t* _data;
  uint32_t _length;

  ReadBlockResult(bool isSuccess, PlatformError error, uint8_t* data,
                  uint32_t length)
      : _isSuccess(isSuccess), _error(error), _data(data), _length(length) {}
 public:
  inline bool IsSucess() { return _isSuccess; }

  inline Block GetBlock() { return {_data, _length}; }

  void ThrowException(v8::Isolate* isolate) {
    if (!_isSuccess) {
      _error.ThrowException(isolate);
    }
  }

  static ReadBlockResult Success(uint8_t* data, uint32_t length) {
    return {true, PlatformError::NoError(), data, length};
  }

  static ReadBlockResult Error() {
    return {false, PlatformError::LastError(), nullptr, 0};
  }
};

class BlockReader {
 public:
  BlockReader() {
  }
  ~BlockReader();

  PlatformOperationStatus Open(v8::Isolate* isolate,
                               v8::Local<v8::String> pathValue);
  ReadBlockResult ReadBlock();

 private:
  uint8_t* _buffer = nullptr;

#ifdef _WIN32
  HANDLE _fileHandle = INVALID_HANDLE_VALUE;
#elif defined(unix)
  int _fd = -1;
#endif
};