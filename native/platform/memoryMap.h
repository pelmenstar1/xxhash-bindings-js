#include <v8.h>

#include <cstdint>

#include "files.h"
#include "platformOperationStatus.h"

#ifdef _WIN32
#include <windows.h>
#endif

class MemoryMappedFile {
 public:
  MemoryMappedFile() {}
  MemoryMappedFile(const MemoryMappedFile& other) = delete;
  ~MemoryMappedFile();

  PlatformOperationStatus Open(v8::Isolate* isolate, const FileOpenOptions& options);

  inline const uint8_t* GetAddress() { return _address; }

  inline size_t GetSize() { return _size; }

 private:
  const uint8_t* _address = nullptr;
  size_t _size = 0;

#ifdef _WIN32
  HANDLE _fileHandle;
  HANDLE _fileMapping;
#endif
};