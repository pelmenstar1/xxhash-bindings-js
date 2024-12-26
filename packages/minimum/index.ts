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

type SeedCheck<S> = {
  expectedType: string;
  is(value: unknown): value is S;
};

let addon: any;
const require = createRequire(import.meta.url);

try {
  addon = require('./build/Release/xxhash-min.node');
} catch {
  addon = require('./build/Debug/xxhash-min.node');
}

function maybeBigintMin(a: number, b: UInt64): number {
  const result = Math.min(a, Number(b));

  return Number.isNaN(result) ? a : result;
}

function maybeBigintMinus(a: UInt64, b: UInt64): UInt64 {
  return typeof a == 'number' && typeof b == 'number'
    ? a - b
    : BigInt(a) - BigInt(b);
}

function createFileHasher<S, H extends UInt64>(
  createState: XxHashVariant<S, H>['createState'],
  seedCheck: SeedCheck<S>,
): XxHashVariant<S, H>['file'] {
  return ({ path, offset, length, seed, preferMap }) => {
    if (preferMap !== undefined && typeof preferMap !== 'boolean') {
      throw new TypeError(
        'Expected type of the property "preferMap" is boolean or undefined',
      );
    }

    if (seed !== undefined && !seedCheck.is(seed)) {
      throw new TypeError(
        `Expected type of the property "seed" is ${seedCheck.expectedType} or undefined`,
      );
    }

    const fd = fs.openSync(path, fs.constants.O_RDONLY);

    try {
      const state = createState(seed);

      const buffer = Buffer.allocUnsafe(maybeBigintMin(4096, length));

      offset = offset ?? 0;

      let currentOffset = offset;

      while (true) {
        const bytesToRead =
          length === undefined
            ? buffer.length
            : maybeBigintMin(
                buffer.length,
                maybeBigintMinus(
                  length,
                  maybeBigintMinus(currentOffset, offset),
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
  seedCheck: SeedCheck<S>,
): XxHashVariant<S, H> {
  return {
    oneshot,
    createState,
    file: createFileHasher(createState, seedCheck),
  };
}

const numberSeedCheck: SeedCheck<number> = {
  expectedType: 'number',
  is: (value) => typeof value == 'number',
};

const uint64SeedCheck: SeedCheck<UInt64> = {
  expectedType: 'number, bigint',
  is: (value) => typeof value == 'number' || typeof value == 'bigint',
};

export const xxhash32 = xxHashVariant<number, number>(
  addon.xxhash32_oneshot,
  addon.xxhash32_createState,
  numberSeedCheck,
);

export const xxhash64 = xxHashVariant<UInt64, bigint>(
  addon.xxhash64_oneshot,
  addon.xxhash64_createState,
  uint64SeedCheck,
);

export const xxhash3 = xxHashVariant<UInt64, bigint>(
  addon.xxhash3_oneshot,
  addon.xxhash3_createState,
  uint64SeedCheck,
);

export const xxhash3_128 = xxHashVariant<UInt64, bigint>(
  addon.xxhash3_128_oneshot,
  addon.xxhash3_128_createState,
  uint64SeedCheck,
);

export default { xxhash32, xxhash64, xxhash3, xxhash3_128 };
