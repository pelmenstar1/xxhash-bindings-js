export type UInt64 = number | bigint;
export type XxVariantName = 'xxhash32' | 'xxhash64' | 'xxhash3' | 'xxhash3_128';

export type AcceptFile = (name: string) => boolean;
export type OnFile<H> = (name: string, hash: H) => void;

export type FileHashingOptions<S> = {
  path: string;
  seed?: S;
  offset?: UInt64;
  length?: UInt64;
  preferMap?: boolean;
};

export type DirectoryHashingOptions<S> = {
  path: string;
  seed?: S;
  preferMap?: boolean;

  acceptFile?: AcceptFile;
};

export type XxHashState<R extends UInt64> = {
  update(data: Uint8Array): void;
  reset(): void;

  result(): R;
};

export type XxHashVariant<S, H extends UInt64> = {
  oneshot(data: Uint8Array, seed?: S): H;
  createState(seed?: S): XxHashState<H>;

  file(options: FileHashingOptions<S>): H;

  directory(options: DirectoryHashingOptions<S> & { onFile: OnFile<H> }): void;
  directoryToMap(options: DirectoryHashingOptions<S>): Map<string, H>;
};
