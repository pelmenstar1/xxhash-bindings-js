#include "fileHashWorker.h"

GenericHashResult BlockHashWorker::Process(const HashWorkerContext& context) {
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

GenericHashResult MapHashWorker::Process(const HashWorkerContext& context) {
  MemoryMappedFile file;
  bool isCompatible = file.Open(context.path, context.offset, context.length);

  if (!isCompatible) {
    // Based on the assumption that the incompatible file is a pretty rare
    // thing, and there's no sense preserving full-fledged BlockHashWorker state
    // inside a MapHashWorker.
    //
    // Use a oneshot method.
    return _HashFile<BlockHashWorker>(context, _variant, _seed);
  }

  size_t size = file.GetSize();
  GenericHashResult result;

  file.Access(
      [&](const uint8_t* address) {
        result = XxHashDynamicState::Oneshot(_variant, address, size, _seed);
      },
      [&] {
        throw std::runtime_error("IO error occurred while reading the file");
      });

  return result;
}
