import { createRequire } from 'module';
import fs from 'fs';
import path from 'path';

import type {
  UInt64,
  XxHashVariant,
  XxHashState,
  AcceptFile,
  OnFile,
} from 'xxhash-bindings-types';

export type {
  DirectoryHashingOptions,
  FileHashingOptions,
  XxHashState,
  XxHashVariant,
  XxVariantName,
} from 'xxhash-bindings-types';

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

function checkPreferMap(value: unknown): asserts value is boolean | undefined {
  if (value !== undefined && typeof value !== 'boolean') {
    throw new TypeError(
      'Expected type of the property "preferMap" is boolean or undefined',
    );
  }
}

function checkSeed<S>(
  value: unknown,
  check: SeedCheck<S>,
): asserts value is S | undefined {
  if (value !== undefined && !check.is(value)) {
    throw new TypeError(
      `Expected type of the property "seed" is ${check.expectedType} or undefined`,
    );
  }
}

function checkAcceptFile(value: unknown): asserts value is Function {
  if (value !== undefined && typeof value !== 'function') {
    throw new TypeError(
      'Expected type of the property "acceptFile" is function or undefined',
    );
  }
}

function createFileHasher<S, H extends UInt64>(
  createState: XxHashVariant<S, H>['createState'],
  seedCheck: SeedCheck<S>,
): XxHashVariant<S, H>['file'] {
  return ({ path, offset, length, seed, preferMap }) => {
    checkSeed(seed, seedCheck);
    checkPreferMap(preferMap);

    const buffer = Buffer.allocUnsafe(maybeBigintMin(4096, length));
    const state = createState(seed);

    return hashFile(path, offset, length, state, buffer);
  };
}

function hashFile<H extends UInt64>(
  path: string,
  offset: UInt64 | undefined,
  length: UInt64 | undefined,
  state: XxHashState<H>,
  buffer: Buffer,
): H {
  const fd = fs.openSync(path, fs.constants.O_RDONLY);

  try {
    offset = offset ?? 0;

    let currentOffset = offset;

    while (true) {
      const bytesToRead =
        length === undefined
          ? buffer.length
          : maybeBigintMin(
              buffer.length,
              maybeBigintMinus(length, maybeBigintMinus(currentOffset, offset)),
            );

      const bytesRead = fs.readSync(fd, buffer, 0, bytesToRead, currentOffset);

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
}

function hashDirectory<S, H extends UInt64>(
  dirPath: string,
  state: XxHashState<H>,
  acceptFile: AcceptFile | undefined,
  onFile: OnFile<H>,
) {
  let dir: fs.Dir | undefined;

  try {
    dir = fs.opendirSync(dirPath);
    const buffer = Buffer.allocUnsafe(4096);

    while (true) {
      const entry = dir.readSync();
      if (entry == null) {
        break;
      }

      if (!entry.isDirectory()) {
        const { name } = entry;

        if (acceptFile === undefined || acceptFile(name)) {
          state.reset();

          const fullPath = path.join(dirPath, name);
          const fileResult = hashFile(fullPath, 0, undefined, state, buffer);

          onFile(name, fileResult);
        }
      }
    }
  } finally {
    dir?.closeSync();
  }
}

function createDirectoryHasher<S, H extends UInt64>(
  createState: XxHashVariant<S, H>['createState'],
  seedCheck: SeedCheck<S>,
): XxHashVariant<S, H>['directory'] {
  return ({ path: dirPath, seed, preferMap, acceptFile, onFile }) => {
    checkSeed(seed, seedCheck);
    checkPreferMap(preferMap);
    checkAcceptFile(acceptFile);

    const state = createState(seed);
    hashDirectory(dirPath, state, acceptFile, onFile);
  };
}

function createDirectoryToMapHasher<S, H extends UInt64>(
  createState: XxHashVariant<S, H>['createState'],
  seedCheck: SeedCheck<S>,
): XxHashVariant<S, H>['directoryToMap'] {
  return ({ path: dirPath, seed, preferMap, acceptFile }) => {
    checkSeed(seed, seedCheck);
    checkPreferMap(preferMap);
    checkAcceptFile(acceptFile);

    const resultMap = new Map();

    const state = createState(seed);
    hashDirectory(dirPath, state, acceptFile, (name, value) =>
      resultMap.set(name, value),
    );

    return resultMap;
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
    directory: createDirectoryHasher(createState, seedCheck),
    directoryToMap: createDirectoryToMapHasher(createState, seedCheck),
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
