#include "fileHashWorker.h"

template <int Variant>
XxResult<Variant> BlockHashWorker<Variant>::Process(
    const HashWorkerContext& context) {
  _state.Reset(_seed);
  _blockReader.Open(context.path, context.offset, context.length);

  while (true) {
    auto block = _blockReader.ReadBlock();

    if (block.length == 0) {
      break;
    }

    _state.Update(block.data, block.length);
  }

  return _state.GetResult();
}

template <int Variant>
XxResult<Variant> MapHashWorker<Variant>::Process(
    const HashWorkerContext& context) {
  MemoryMappedFile file;
  bool isCompatible = file.Open(context.path, context.offset, context.length);

  if (!isCompatible) {
    // Based on the assumption that the incompatible file is a pretty rare
    // thing, and there's no sense preserving full-fledged BlockHashWorker state
    // inside a MapHashWorker.
    //
    // Use a oneshot method.
    return _HashFile<Variant, BlockHashWorker<Variant>>(context, _seed);
  }

  size_t size = file.GetSize();
  XxResult<Variant> result;

  file.Access(
      [&](const uint8_t* address) {
        result = XxHashTraits<Variant>::Oneshot(address, size, _seed);
      },
      [&] {
        throw std::runtime_error("IO error occurred while reading the file");
      });

  return result;
}

#define _INSTANTIATE_WORKER(name, variant) template class name<variant>;

#define INSTANTIATE_WORKER(name) \
  _INSTANTIATE_WORKER(name, H32) \
  _INSTANTIATE_WORKER(name, H64) \
  _INSTANTIATE_WORKER(name, H3)  \
  _INSTANTIATE_WORKER(name, H3_128)

INSTANTIATE_WORKER(BlockHashWorker)
INSTANTIATE_WORKER(MapHashWorker)
