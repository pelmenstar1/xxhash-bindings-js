#include "directoryIterator.h"

#include <iostream>
#include <stdexcept>

#include "fullPathBuilder.h"
#include "platformError.h"

#if defined(__linux__) || defined(__APPLE__)
#include <sys/stat.h>
#endif

#define CHECK_FIND_ERROR(cond, endError)  \
  if (cond) {                             \
    DWORD lastError = GetLastError();     \
    if (lastError == endError) {          \
      _hasMore = false;                   \
    } else {                              \
      throw PlatformException(lastError); \
    }                                     \
  }

template <typename CharType>
DirectoryIterator<CharType>::DirectoryIterator(const NativeString& path)
    : _pathBuilder(path) {
#ifdef _WIN32
  _hasMore = true;
  _pathBuilder.SetNativeSearchPattern();
  auto nativeBuffer = _pathBuilder.NativeBuffer();

  _hFind = FindFirstFileW(nativeBuffer, &_findData);
  CHECK_FIND_ERROR(_hFind == INVALID_HANDLE_VALUE, ERROR_FILE_NOT_FOUND)
#else
  _dir = opendir(path.c_str());
  CHECK_PLATFORM_ERROR(_dir == nullptr)
#endif
}

template <typename CharType>
DirectoryIterator<CharType>::~DirectoryIterator() {
#ifdef _WIN32
  if (_hFind != INVALID_HANDLE_VALUE) {
    FindClose(_hFind);
  }
#else
  if (_dir != nullptr) {
    closedir(_dir);
  }
#endif
}

template <typename CharType>
bool DirectoryIterator<CharType>::Next(DirectoryEntry<CharType>* entry) {
  while (true) {
#ifdef _WIN32
    if (!_hasMore) {
      return false;
    }

    bool found = false;

    // Skip all directories.
    if ((_findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0) {
      _pathBuilder.SetNativeFileName(_findData.cFileName);

      entry->fullPath = _pathBuilder.Buffer();
      entry->fileName = _pathBuilder.GetFileName();
      
      found = true;
    }

    bool result = FindNextFileW(_hFind, &_findData);
    CHECK_FIND_ERROR(!result, ERROR_NO_MORE_FILES)

    if (found) {
      return true;
    }
#else
    errno = 0;

    dirent* ent = readdir(_dir);
    if (ent == nullptr) {
      // It's the end of the directory
      if (errno == 0) {
        return false;
      }

      ThrowPlatformException();
    }

    char type = ent->d_type;

    // Skip the directories.
    if (type == DT_DIR) {
      continue;
    }

    _pathBuilder.SetNativeFileName(ent->d_name);

    if (ent->d_type == DT_UNKNOWN) {
      struct stat statResult;
      CHECK_PLATFORM_ERROR(stat(_pathBuilder.NativeBuffer(), &statResult))

      // Skip the directories.
      if (S_ISDIR(statResult.st_mode)) {
        continue;
      }
    }

    entry->fullPath = _pathBuilder.Buffer();
    entry->fileName = _pathBuilder.GetFileName();

    return true;
#endif
  }
}

template class DirectoryIterator<char>;
template class DirectoryIterator<wchar_t>;
