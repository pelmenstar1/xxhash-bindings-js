#pragma once

#include <cstdint>

#include "nativeString.h"

template <typename CharType>
class NativeBufferPair {
 public:
  NativeBufferPair() {}
  NativeBufferPair(NativeChar* buffer, size_t dirPartLength,
                   size_t bufferLength);
  NativeBufferPair(const NativeBufferPair<CharType>& other) = delete;
  NativeBufferPair(NativeBufferPair<CharType>&& other) {
    _buffer = other._buffer;
    _nativeBuffer = other._nativeBuffer;
    _dirPartLength = other._dirPartLength;
    _bufferLength = other._bufferLength;

    other._buffer = nullptr;
    other._nativeBuffer = nullptr;
  }

  ~NativeBufferPair();

  NativeBufferPair<CharType>& operator=(NativeBufferPair<CharType>&& other) {
    _buffer = other._buffer;
    _nativeBuffer = other._nativeBuffer;
    _dirPartLength = other._dirPartLength;
    _bufferLength = other._bufferLength;

    other._buffer = nullptr;
    other._nativeBuffer = nullptr;

    return *this;
  }

  CharType* Buffer() const { return _buffer; }
  NativeChar* NativeBuffer() const { return _nativeBuffer; }
  size_t BufferLength() const { return _bufferLength; }
  size_t DirectoryPartLength() const { return _dirPartLength; }

  void SetFileName(const NativeChar* text);

 private:
  CharType* _buffer = 0;
  NativeChar* _nativeBuffer = 0;
  size_t _dirPartLength = 0;
  size_t _bufferLength = 0;

  void Resize(size_t newSize);
};

template <typename CharType>
class FullPathBuilder {
 public:
  FullPathBuilder(const NativeChar* path);
  FullPathBuilder(const NativeString& path)
      : FullPathBuilder<CharType>(path.c_str()) {}
  FullPathBuilder(const FullPathBuilder<CharType>& other) = delete;

  void SetNativeFileName(const NativeChar* name) {
    _bufferPair.SetFileName(name);
  }

  CharType* GetFileName() const {
    return _bufferPair.Buffer() + _bufferPair.DirectoryPartLength();
  }

  void SetNativeSearchPattern() {
    NativeChar* buffer = _bufferPair.NativeBuffer();
    size_t dirPartLength = _bufferPair.DirectoryPartLength();

    // Expects that _buffer is larger than _dirPartLength
    buffer[dirPartLength] = '*';
    buffer[dirPartLength + 1] = '\0';
  }

  NativeChar* NativeBuffer() const { return _bufferPair.NativeBuffer(); }
  CharType* Buffer() const { return _bufferPair.Buffer(); }
  uint32_t BufferLength() const { return _bufferPair.BufferLength(); }

 private:
  NativeBufferPair<CharType> _bufferPair;
};
