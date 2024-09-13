type UInt64 = number | bigint;

export declare enum FileHashingType {
  MAP = 0,
  BLOCK = 1
}

export type FileHashingOptions<S> = {
  path: string,
  seed?: S,
  type?: FileHashingType
}

type XxHashVariant<S, H> = {
  oneshot(data: Uint8Array, seed?: S): H;
  file(options: FileHashingOptions<S>): H;
}

type XxHash3 = XxHashVariant<UInt64, bigint>;

export declare const xxhash32: XxHashVariant<number, number>;
export declare const xxhash64: XxHashVariant<UInt64, bigint>;

export declare const xxhash3: XxHash3;
export declare const xxhash3_128: XxHash3;