import bindings from 'bindings';

type UInt64 = number | bigint;

export type FileHashingOptions<S> = {
  path: string;
  seed?: S;
  offset?: UInt64;
  length?: UInt64;
  preferMap?: boolean;
};

type XxHashVariant<S, H extends UInt64> = {
  oneshot(data: Uint8Array, seed?: S): H;
  file(options: FileHashingOptions<S>): H;

  createState(seed?: S): XxHashState<H>;
};

type NativeFileHasher<S, H extends UInt64> = (
  path: string,
  seed: S,
  preferMap: boolean,
  offset: UInt64,
  length: UInt64,
) => H;

export type XxHashState<R extends UInt64> = {
  update(data: Uint8Array): void;

  result(): R;
};

const addon = bindings('xxhash');

function modifiedFileHasher<S, H extends UInt64>(
  hasher: NativeFileHasher<S, H>,
): XxHashVariant<S, H>['file'] {
  return (options) => {
    return hasher(
      options.path,
      options.seed,
      options.preferMap,
      options.offset,
      options.length,
    );
  };
}

function xxHashVariant<S, H extends UInt64>(
  oneshot: XxHashVariant<S, H>['oneshot'],
  file: NativeFileHasher<S, H>,
  createState: XxHashVariant<S, H>['createState'],
): XxHashVariant<S, H> {
  return {
    oneshot,
    createState,
    file: modifiedFileHasher(file),
  };
}

export const xxhash32 = xxHashVariant<number, number>(
  addon.xxhash32_oneshot,
  addon.xxhash32_file,
  addon.xxhash32_createState,
);

export const xxhash64 = xxHashVariant<UInt64, bigint>(
  addon.xxhash64_oneshot,
  addon.xxhash64_file,
  addon.xxhash64_createState,
);

export const xxhash3 = xxHashVariant<UInt64, bigint>(
  addon.xxhash3_oneshot,
  addon.xxhash3_file,
  addon.xxhash3_createState,
);

export const xxhash3_128 = xxHashVariant<UInt64, bigint>(
  addon.xxhash3_128_oneshot,
  addon.xxhash3_128_file,
  addon.xxhash3_128_createState,
);
