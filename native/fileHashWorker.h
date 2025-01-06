#include "hashers.h"
#include "platform/blockReader.h"
#include "platform/memoryMap.h"
#include "platform/nativeString.h"
#include "platform/platformError.h"

template <typename CharType = NativeChar>
struct HashWorkerContext {
  const CharType* path;
  size_t offset;
  size_t length;

  HashWorkerContext(const CharType* path, size_t offset, size_t length)
      : path(path), offset(offset), length(length) {}
};

template <int Variant>
class HashWorker {
 public:
  virtual XxResult<Variant> Process(const HashWorkerContext<>& context) = 0;
};

template <int Variant>
class BlockHashWorker : public HashWorker<Variant> {
 public:
  BlockHashWorker(XxSeed<Variant> seed) : _seed(seed) {}

  XxResult<Variant> Process(const HashWorkerContext<>& context) override;
 private:
  BlockReader _blockReader;
  XxHashState<Variant> _state;
  XxSeed<Variant> _seed;
};

template <int Variant>
class MapHashWorker : public HashWorker<Variant> {
 public:
  MapHashWorker(XxSeed<Variant> seed) : _seed(seed) {}

  XxResult<Variant> Process(const HashWorkerContext<>& context) override;

 private:
  XxSeed<Variant> _seed;
};

template <int Variant, typename Worker>
XxResult<Variant> _HashFile(const HashWorkerContext<>& context,
                            XxSeed<Variant> seed) {
  Worker worker(seed);

  return worker.Process(context);
}

template <int Variant>
XxResult<Variant> HashFile(const HashWorkerContext<>& context,
                           XxSeed<Variant> seed, bool preferMap) {
  return preferMap
             ? _HashFile<Variant, MapHashWorker<Variant>>(context, seed)
             : _HashFile<Variant, BlockHashWorker<Variant>>(context, seed);
}
