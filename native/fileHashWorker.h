#include "hashers.h"
#include "platform/blockReader.h"
#include "platform/memoryMap.h"
#include "platform/nativeString.h"
#include "platform/platformError.h"

#define _INSTANTIATE_WORKER(name, variant) template class name<variant>;

#define INSTANTIATE_WORKER(name) \
  _INSTANTIATE_WORKER(name, H32) \
  _INSTANTIATE_WORKER(name, H64) \
  _INSTANTIATE_WORKER(name, H3)  \
  _INSTANTIATE_WORKER(name, H3_128)

template <int Variant>
struct FileHashingContext {
  const NativeChar* path;
  size_t offset;
  size_t length;
  XxSeed<Variant> seed;

  FileHashingContext(const NativeChar* path, size_t offset, size_t length,
                     XxSeed<Variant> seed)
      : path(path), offset(offset), length(length), seed(seed) {}
};

template <int Variant>
class HashWorker {
 public:
  virtual XxResult<Variant> Process(const NativeChar* path, size_t offset,
                                    size_t length) = 0;
};

template <int Variant>
class BlockHashWorker : public HashWorker<Variant> {
 public:
  BlockHashWorker(XxSeed<Variant> seed) : _seed(seed) {}

  XxResult<Variant> Process(const NativeChar* path, size_t offset,
                            size_t length) override;

 private:
  BlockReader _blockReader;
  XxHashState<Variant> _state;
  XxSeed<Variant> _seed;
};

template <int Variant>
class MapHashWorker : public HashWorker<Variant> {
 public:
  MapHashWorker(XxSeed<Variant> seed) : _seed(seed) {}

  XxResult<Variant> Process(const NativeChar* path, size_t offset,
                            size_t length) override;

 private:
  XxSeed<Variant> _seed;
};

INSTANTIATE_WORKER(BlockHashWorker)
INSTANTIATE_WORKER(MapHashWorker)

template <int Variant, typename Worker>
XxResult<Variant> _HashFile(const FileHashingContext<Variant>& context) {
  Worker worker(context.seed);

  return worker.Process(context.path, context.offset, context.length);
}

template <int Variant>
XxResult<Variant> HashFile(const FileHashingContext<Variant>& context,
                           bool preferMap) {
  return preferMap ? _HashFile<Variant, MapHashWorker<Variant>>(context)
                   : _HashFile<Variant, BlockHashWorker<Variant>>(context);
}
