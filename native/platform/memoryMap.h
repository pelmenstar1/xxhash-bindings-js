#include <v8.h>

#include <cstdint>
#include "platformOperationStatus.h"

#ifdef _WIN32
#include <windows.h>
#endif

class MemoryMappedFile {
 public:
  MemoryMappedFile() {}
  MemoryMappedFile(const MemoryMappedFile& other) = delete;
  ~MemoryMappedFile();

  PlatformOperationStatus Open(v8::Isolate* isolate,
                           v8::Local<v8::String> pathValue);

  inline const uint8_t* GetAddress() { return _address; }

  inline uint64_t GetSize() { return _size; }

 private:
  const uint8_t* _address;
  uint64_t _size;

#ifdef _WIN32
  HANDLE _fileHandle;
  HANDLE _fileMapping;
#endif
};