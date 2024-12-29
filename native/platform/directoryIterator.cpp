#include "directoryIterator.h"

#include <iostream>
#include <stdexcept>

#include "platformError.h"

#ifdef unix
#include <sys/stat.h>
#endif

#ifdef _WIN32
wchar_t* GetFullPathEnsure(const NativeString& path, uint32_t* pathLength,
                           uint32_t* bufferLength) {
  uint32_t bufLength = 128;

  auto buffer = new wchar_t[bufLength];
  DWORD result = GetFullPathNameW(path.c_str(), bufLength, buffer, NULL);

  if (result > bufLength - 2) {
    delete[] buffer;

    bufLength = result + 128;
    buffer = new wchar_t[bufLength];
    result = GetFullPathNameW(path.c_str(), result, buffer, NULL);
  }

  if (result == 0) {
    delete[] buffer;
    ThrowPlatformException();
  }

  buffer[result] = '\\';
  buffer[result + 1] = '\0';

  *pathLength = result + 1;
  *bufferLength = bufLength;

  return buffer;
}
#endif

DirectoryIterator::DirectoryIterator(const NativeString& path) {
#ifdef _WIN32
  _filePathBuffer =
      GetFullPathEnsure(path, &_dirPartLength, &_filePathBufferLength);

  WIN32_FILE_ATTRIBUTE_DATA attribData;
  CHECK_PLATFORM_ERROR(!GetFileAttributesExW(
      _filePathBuffer, GetFileExInfoStandard, &attribData))

  if ((attribData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0) {
    throw std::runtime_error("Given path is not a directory");
  }
#else
  _dir = opendir(path.c_str());
  CHECK_PLATFORM_ERROR(_dir == nullptr)

  char* fullPath = realpath(path.c_str(), NULL);
  CHECK_PLATFORM_ERROR(fullPath == nullptr)

  size_t fullPathLength = strlen(fullPath);

  _filePathBufferLength = fullPathLength + 128;
  _filePathBuffer = new char[_filePathBufferLength];

  memcpy(_filePathBuffer, fullPath, fullPathLength);
  _filePathBuffer[fullPathLength] = '/';

  _dirPartLength = fullPathLength + 1;

  free(fullPath);
#endif
}

DirectoryIterator::~DirectoryIterator() {
  delete[] _filePathBuffer;

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

void DirectoryIterator::AppendFileNameToBuffer(const NativeChar* fileName) {
  size_t fileNameLength = NativeStringLen(fileName);
  size_t totalLength = _dirPartLength + fileNameLength;

  if (_filePathBufferLength > totalLength) {
#ifdef _WIN32
    size_t newBufferLength = _dirPartLength + MAX_PATH;
#else
    size_t newBufferLength = totalLength + 64;
#endif

    auto newBuffer = new NativeChar[newBufferLength + 1];
    memcpy(newBuffer, _filePathBuffer, _dirPartLength * sizeof(NativeChar));

    delete[] _filePathBuffer;
    _filePathBuffer = newBuffer;
    _filePathBufferLength = newBufferLength;
  }

  memcpy(_filePathBuffer + _dirPartLength, fileName,
         fileNameLength * sizeof(NativeChar));
  _filePathBuffer[totalLength] = 0;
}

bool DirectoryIterator::Next(DirectoryEntry* entry) {
  while (true) {
#ifdef _WIN32
    bool isInvalid;
    DWORD endOfDirError;

    if (_hFind == INVALID_HANDLE_VALUE) {
      _filePathBuffer[_dirPartLength] = '*';
      _filePathBuffer[_dirPartLength + 1] = '\0';

      _hFind = FindFirstFileW(_filePathBuffer, &_findData);

      isInvalid = _hFind == INVALID_HANDLE_VALUE;
      endOfDirError = ERROR_FILE_NOT_FOUND;
    } else {
      bool result = FindNextFileW(_hFind, &_findData);

      isInvalid = !result;
      endOfDirError = ERROR_NO_MORE_FILES;
    }

    if (isInvalid) {
      DWORD lastError = GetLastError();

      if (lastError == endOfDirError) {
        return false;
      }

      ThrowPlatformException();
    }

    // Skip all directories.
    if ((_findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0) {
      wchar_t* fileName = _findData.cFileName;
      AppendFileNameToBuffer(fileName);

      entry->fullPath = _filePathBuffer;
      entry->fileName = fileName;

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

    char* fileName = ent->d_name;

    AppendFileNameToBuffer(fileName);

    if (ent->d_type == DT_UNKNOWN) {
      struct stat statResult;
      CHECK_PLATFORM_ERROR(stat(_filePathBuffer, &statResult))

      // Skip the directories.
      if (S_ISDIR(statResult.st_mode)) {
        continue;
      }
    }

    entry->fullPath = _filePathBuffer;
    entry->fileName = fileName;

    return true;
#endif
  }
}
