#pragma once

#include <cstdint>

#include "nativeString.h"

#ifdef _WIN32
#include <windows.h>
#else
#include <dirent.h>
#include <sys/types.h>
#endif

class DirectoryEntry {
 public:
  const NativeChar* fullPath;
  const NativeChar* fileName;
};

class DirectoryIterator {
 public:
  DirectoryIterator(const NativeString& path);
  DirectoryIterator(const DirectoryIterator& other) = delete;
  ~DirectoryIterator();

  bool Next(DirectoryEntry* entry);

 private:
  void AppendFileNameToBuffer(const NativeChar* fileName);
  
#ifdef _WIN32
  HANDLE _hFind = INVALID_HANDLE_VALUE;
  WIN32_FIND_DATAW _findData;
#else
  DIR* _dir;
#endif

  uint32_t _dirPartLength;

  NativeChar* _filePathBuffer;
  uint32_t _filePathBufferLength;
};
