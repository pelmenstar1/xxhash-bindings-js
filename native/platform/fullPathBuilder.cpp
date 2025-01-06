#include "fullPathBuilder.h"

#include <uv.h>

#include "platformError.h"

#ifdef _WIN32

#include <Windows.h>
#undef max

#endif

template <>
NativeBufferPair<NativeChar>::NativeBufferPair(NativeChar* buffer,
                                               size_t dirPartLength,
                                               size_t bufferLength)
    : _buffer(buffer),
      _nativeBuffer(buffer),
      _dirPartLength(dirPartLength),
      _bufferLength(bufferLength) {}

template <typename CharType>
NativeBufferPair<CharType>::~NativeBufferPair() {
  delete[] _buffer;
  delete[] _nativeBuffer;
}

template <>
NativeBufferPair<NativeChar>::~NativeBufferPair() {
  delete[] _buffer;
}

template <typename CharType>
void NativeBufferPair<CharType>::Resize(size_t minimumSize) {
#ifdef _WIN32
  size_t newBufferLength = std::max(minimumSize, _dirPartLength + MAX_PATH);
#else
  size_t newBufferLength = minimumSize + 64;
#endif

  CharType* newBuffer = new CharType[newBufferLength + 1];
  memcpy(newBuffer, _buffer, _dirPartLength * sizeof(CharType));

  delete[] _buffer;
  _buffer = newBuffer;
  _bufferLength = newBufferLength;
}

// Only wchar_t -> char conversion on Windows is supported
// Currently char -> wchar_t on non-Windows platform is not needed.
#ifdef _WIN32

NativeBufferPair<char>::NativeBufferPair(wchar_t* buffer, size_t dirPartLength,
                                         size_t bufferLength)
    : _nativeBuffer(buffer), _dirPartLength(dirPartLength) {
  int requiredSize = WideCharToMultiByte(CP_UTF8, WC_ERR_INVALID_CHARS, buffer,
                                         dirPartLength, NULL, 0, NULL, NULL);
  CHECK_PLATFORM_ERROR(requiredSize == 0)

  _bufferLength = requiredSize;
  _buffer = new char[requiredSize + 1];
  int result =
      WideCharToMultiByte(CP_UTF8, WC_ERR_INVALID_CHARS, buffer, dirPartLength,
                          _buffer, _bufferLength, NULL, NULL);
  CHECK_PLATFORM_ERROR(result == 0)
}

void NativeBufferPair<char>::SetFileName(const wchar_t* fileName) {
  size_t fileNameLength = NativeStringLen(fileName);
  size_t requiredLength = _dirPartLength + fileNameLength;

  if (requiredLength > _bufferLength) {
    Resize(requiredLength);
  }

  int result = WideCharToMultiByte(CP_UTF8, WC_ERR_INVALID_CHARS, fileName,
                                   fileNameLength, _buffer + _dirPartLength,
                                   _bufferLength - _dirPartLength, NULL, NULL);
  CHECK_PLATFORM_ERROR(result == 0);

  _buffer[requiredLength] = '\0';
}
#endif

template <>
void NativeBufferPair<NativeChar>::SetFileName(const NativeChar* fileName) {
  size_t fileNameLength = NativeStringLen(fileName);
  size_t requiredLength = _dirPartLength + fileNameLength;

  if (requiredLength > _bufferLength) {
    Resize(requiredLength);
  }

  memcpy(_buffer + _dirPartLength, fileName,
         fileNameLength * sizeof(NativeChar));
  _buffer[_dirPartLength + fileNameLength] = '\0';
}

template <typename CharType>
FullPathBuilder<CharType>::FullPathBuilder(const NativeChar* path) {
#ifdef _WIN32
  uint32_t bufLength = 128;

  auto buffer = new wchar_t[bufLength];
  DWORD result = GetFullPathNameW(path, bufLength, buffer, NULL);

  if (result > bufLength - 2) {
    delete[] buffer;

    bufLength = result + 128;
    buffer = new wchar_t[bufLength];
    result = GetFullPathNameW(path, result, buffer, NULL);
  }

  if (result == 0) {
    delete[] buffer;
    ThrowPlatformException();
  }

  buffer[result] = '\\';
  buffer[result + 1] = '\0';

  _bufferPair =
      std::move(NativeBufferPair<CharType>(buffer, result + 1, bufLength));
#else
  char* fullPath = realpath(path, NULL);
  CHECK_PLATFORM_ERROR(fullPath == nullptr)

  size_t fullPathLength = strlen(fullPath);

  size_t bufferLength = fullPathLength + 128;

  char* buffer = new char[bufferLength];
  memcpy(buffer, fullPath, fullPathLength);
  buffer[fullPathLength] = '/';

  _bufferPair =
      NativeBufferPair<CharType>(buffer, fullPathLength + 1, bufferLength);

  free(fullPath);
#endif
}

template class FullPathBuilder<char>;
template class FullPathBuilder<wchar_t>;
