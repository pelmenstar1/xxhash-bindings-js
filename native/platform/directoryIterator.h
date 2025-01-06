#pragma once

#include <cstdint>

#include "nativeString.h"
#include "platformError.h"
#include "fullPathBuilder.h"

#ifdef _WIN32
#include <windows.h>
#else
#include <dirent.h>
#include <sys/types.h>
#endif

template<typename CharType = NativeChar>
struct DirectoryEntry {
  const CharType* fullPath;
  const CharType* fileName;
};

template<typename CharType = NativeChar>
class DirectoryIterator {
 public:
  DirectoryIterator(const NativeString& path);
  DirectoryIterator(const DirectoryIterator<CharType>& other) = delete;
  ~DirectoryIterator();

  bool Next(DirectoryEntry<CharType>* entry);

 private:
#ifdef _WIN32
  HANDLE _hFind = INVALID_HANDLE_VALUE;
  WIN32_FIND_DATAW _findData;
  bool _hasMore;
#else
  DIR* _dir;
#endif

  FullPathBuilder<CharType> _pathBuilder;
};
