type UInt64 = number | bigint;

export declare enum FileHashingType {
  MAP = 0,
  BLOCK = 1
}

type XxHash3 = {
  oneshot(data: Uint8Array, seed?: UInt64): bigint;
  file(path: string, seed?: UInt64, type?: FileHashingType): bigint;
}

export declare const xxhash32: {
  oneshot(data: Uint8Array, seed: number): number;
  file(path: string, seed: number, type?: FileHashingType): number;
}

export declare const xxhash64: {
  oneshot(data: Uint8Array, seed: UInt64): bigint;
  file(path: string, seed: UInt64, type?: FileHashingType): bigint;
}

export declare const xxhash3: XxHash3;
export declare const xxhash3_128: XxHash3;