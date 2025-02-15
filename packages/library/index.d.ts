type UInt64 = number | bigint;

export type XxVariantName = 'xxhash32' | 'xxhash64' | 'xxhash3' | 'xxhash3_128';

export type FileHashOptions<S> = {
  path: string;
  seed?: S;
  offset?: UInt64;
  length?: UInt64;
  preferMap?: boolean;
};

export type XxHashState<R extends UInt64> = {
  update(data: Uint8Array): void;
  reset(): void;

  result(): R;
};

export type XxHashVariant<S, H extends UInt64> = {
  oneshot(data: Uint8Array, seed?: S): H;
  createState(seed?: S): XxHashState<H>;

  file(options: FileHashOptions<S>): H;
  fileAsync(options: FileHashOptions<S>): Promise<H>;
};

/*
export class XxHashState32 {
  constructor(seed?: number);
  update(data: Uint8Array): void;
  reset(): void;

  result(): number;
}

export class XxHashState64 {
  constructor(seed?: UInt64);
  update(data: Uint8Array): void;
  reset(): void;

  result(): bigint;
}

export class XxHashState3 {
  constructor(seed?: UInt64);
  update(data: Uint8Array): void;
  reset(): void;

  result(): bigint;
}

export class XxHashState3_128 {
  constructor(seed?: UInt64);
  update(data: Uint8Array): void;
  reset(): void;

  result(): bigint;
}
  */

export declare const xxhash32: XxHashVariant<number, number>;
export declare const xxhash64: XxHashVariant<UInt64, bigint>;
export declare const xxhash3: XxHashVariant<UInt64, bigint>;
export declare const xxhash3_128: XxHashVariant<UInt64, bigint>;
declare const _default: {
  xxhash32: XxHashVariant<number, number>;
  xxhash64: XxHashVariant<UInt64, bigint>;
  xxhash3: XxHashVariant<UInt64, bigint>;
  xxhash3_128: XxHashVariant<UInt64, bigint>;
};

export default _default;
