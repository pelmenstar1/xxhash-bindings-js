import { createRequire } from 'module';
import fs from 'fs';

type UInt64 = number | bigint;

export type FileHashingOptions<S> = {
  path: string;
  seed?: S;
  offset?: UInt64;
  length?: UInt64;
  preferMap?: boolean;
};

export type XxHashVariant<S, H extends UInt64> = {
  oneshot(data: Uint8Array, seed?: S): H;
  file(options: FileHashingOptions<S>): H;

  createState(seed?: S): XxHashState<H>;
};

export type XxHashState<R extends UInt64> = {
  update(data: Uint8Array): void;

  result(): R;
};

let addon: any;
const require = createRequire(import.meta.url);

try {
  addon = require('./build/Release/xxhash-min.node');
} catch {
  addon = require('./build/Debug/xxhash-min.node');
}

function maybeBigintMinus(
  a: number | bigint,
  b: number | bigint,
): number | bigint {
  if (typeof a === 'number') {
    return typeof b === 'number' ? a - b : BigInt(a) - b;
  }

  return typeof b === 'number' ? a - BigInt(b) : a - b;
}

function bigintToNumber(a: number | bigint): number {
  return typeof a === 'number' ? a : Number.parseInt(a.toString(), 10);
}

function createFileHasher<S, H extends UInt64>(
  createState: XxHashVariant<S, H>['createState'],
): XxHashVariant<S, H>['file'] {
  return ({ path, offset, length, seed, preferMap }) => {
    if (typeof preferMap !== 'boolean' && typeof preferMap !== 'undefined') {
      throw new TypeError('Expected type of the argument 2 is boolean');
    }

    const fd = fs.openSync(path, fs.constants.O_RDONLY);

    try {
      const state = createState(seed);

      const buffer = Buffer.allocUnsafe(
        length == undefined ? 4096 : Math.min(4096, Number(length)),
      );

      let currentOffset = offset ?? 0;

      while (true) {
        const bytesToRead =
          length === undefined
            ? buffer.length
            : Math.min(
                buffer.length,
                bigintToNumber(
                  maybeBigintMinus(
                    length,
                    maybeBigintMinus(currentOffset, offset),
                  ),
                ),
              );

        const bytesRead = fs.readSync(
          fd,
          buffer,
          0,
          bytesToRead,
          currentOffset,
        );

        if (bytesRead == 0) {
          break;
        }

        state.update(buffer.subarray(0, bytesRead));

        if (typeof currentOffset == 'bigint') {
          currentOffset += BigInt(bytesRead);
        } else {
          currentOffset += bytesRead;
        }
      }

      return state.result();
    } finally {
      fs.closeSync(fd);
    }
  };
}

function xxHashVariant<S, H extends UInt64>(
  oneshot: XxHashVariant<S, H>['oneshot'],
  createState: XxHashVariant<S, H>['createState'],
): XxHashVariant<S, H> {
  return {
    oneshot,
    createState,
    file: createFileHasher(createState),
  };
}

export const xxhash32 = xxHashVariant<number, number>(
  addon.xxhash32_oneshot,
  addon.xxhash32_createState,
);

export const xxhash64 = xxHashVariant<UInt64, bigint>(
  addon.xxhash64_oneshot,
  addon.xxhash64_createState,
);

export const xxhash3 = xxHashVariant<UInt64, bigint>(
  addon.xxhash3_oneshot,
  addon.xxhash3_createState,
);

export const xxhash3_128 = xxHashVariant<UInt64, bigint>(
  addon.xxhash3_128_oneshot,
  addon.xxhash3_128_createState,
);

export default { xxhash32, xxhash64, xxhash3, xxhash3_128 };
