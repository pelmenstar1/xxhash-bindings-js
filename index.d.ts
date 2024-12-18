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

type XxHash3 = XxHashVariant<UInt64, bigint>;

export type XxHashState<R extends UInt64> = {
  update(data: Uint8Array): void;

  result(): R;
};

export declare const xxhash32: XxHashVariant<number, number>;
export declare const xxhash64: XxHashVariant<UInt64, bigint>;

export declare const xxhash3: XxHash3;
export declare const xxhash3_128: XxHash3;
