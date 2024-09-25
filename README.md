## xxhash bindings for NodeJS

The library supports all existing xxhash variants: `xxhash32`, `xxhash64`, `xxhash3` (64 and 128 bits)

## Usage

```typescript
import { xxhash3 /* or any other variant */ } from 'xxhash-bindings-js';

// Oneshot
xxhash3.oneshot(
  data // must be Uint8Array, 
  1 // seed, optional, defaults to 0
)

// Hash entire file
xxhash3.file({
  path: '/path/to/file',
  seed: 1 // optional, defaults to 0
  mode: FileHashingMode.BLOCK // optional, defaults to MAP
})
```

# File hashing mode

There's two ways to read all contents from a file: read block by block, or [map](https://en.wikipedia.org/wiki/Memory-mapped_file) entire file in the memory. `Block` mode is the simplest way to read a block: read a block, hash it, read a next block until end of the file. On the other hand, you can map all the file into virtual memory (it won't actually be in the RAM, but still it will allocate some space), and use it as plain contigious region of memory.

With the current implementation `MAP` mode is generally faster.
