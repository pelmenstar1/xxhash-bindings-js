#pragma once

#include "hashers.h"
#include "platform/blockReader.h"
#include "platform/memoryMap.h"
#include "platform/nativeString.h"
#include "platform/platformError.h"

struct HashWorkerContext {
  NativeString path;
  size_t offset;
  size_t length;

  HashWorkerContext(NativeString path, size_t offset, size_t length)
      : path(path), offset(offset), length(length) {}
};

class HashWorker {
 public:
  virtual GenericHashResult Process(const HashWorkerContext& context) = 0;
};

class BlockHashWorker : public HashWorker {
 public:
  BlockHashWorker(uint32_t variant, uint64_t seed) : _state(variant, seed) {}

  GenericHashResult Process(const HashWorkerContext& context) override;

 private:
  BlockReader _blockReader;
  XxHashDynamicState _state;
};

class MapHashWorker : public HashWorker {
 public:
  MapHashWorker(uint32_t variant, uint64_t seed) : _variant(variant), _seed(seed) {}

  GenericHashResult Process(const HashWorkerContext& context) override;

 private:
  uint32_t _variant;
  uint64_t _seed;
};

template <typename Worker>
inline GenericHashResult _HashFile(const HashWorkerContext& context, uint32_t variant,
                            uint64_t seed) {
  Worker worker(variant, seed);

  return worker.Process(context);
}

inline GenericHashResult HashFile(const HashWorkerContext& context, uint32_t variant,
                           uint64_t seed, bool preferMap) {
  return preferMap ? _HashFile<MapHashWorker>(context, variant, seed)
                   : _HashFile<BlockHashWorker>(context, variant, seed);
}
