import { createRequire } from 'module';

type UInt64 = number | bigint;

export type FileHashingOptions<S> = {
  path: string;
  seed?: S;
  offset?: UInt64;
  length?: UInt64;
  preferMap?: boolean;
};

type AcceptFile = (name: string) => boolean;
type OnFile<H> = (name: string, hash: H) => void;

export type DirectoryHashingOptions<S> = {
  path: string;
  seed?: S;
  preferMap?: boolean;

  acceptFile?: AcceptFile;
};

export type XxHashVariant<S, H extends UInt64> = {
  oneshot(data: Uint8Array, seed?: S): H;
  createState(seed?: S): XxHashState<H>;

  file(options: FileHashingOptions<S>): H;

  directory(options: DirectoryHashingOptions<S> & { onFile: OnFile<H> }): void;
  directoryToMap(options: DirectoryHashingOptions<S>): Map<string, H>;
};

export type XxHashState<R extends UInt64> = {
  update(data: Uint8Array): void;

  result(): R;
};

function xxHashVariant<S, H extends UInt64>(
  oneshot: XxHashVariant<S, H>['oneshot'],
  createState: XxHashVariant<S, H>['createState'],
  file: XxHashVariant<S, H>['file'],
  directory: XxHashVariant<S, H>['directory'],
  directoryToMap: XxHashVariant<S, H>['directoryToMap'],
): XxHashVariant<S, H> {
  return {
    oneshot,
    createState,
    file,
    directory,
    directoryToMap,
  };
}

let addon: any;
const require = createRequire(import.meta.url);

try {
  addon = require('./build/Release/xxhash-allnative.node');
} catch {
  addon = require('./build/Debug/xxhash-allnative.node');
}

export const xxhash32 = xxHashVariant<number, number>(
  addon.xxhash32_oneshot,
  addon.xxhash32_createState,
  addon.xxhash32_file,
  addon.xxhash32_directory,
  addon.xxhash32_directoryToMap,
);

export const xxhash64 = xxHashVariant<UInt64, bigint>(
  addon.xxhash64_oneshot,
  addon.xxhash64_createState,
  addon.xxhash64_file,
  addon.xxhash64_directory,
  addon.xxhash64_directoryToMap,
);

export const xxhash3 = xxHashVariant<UInt64, bigint>(
  addon.xxhash3_oneshot,
  addon.xxhash3_createState,
  addon.xxhash3_file,
  addon.xxhash3_directory,
  addon.xxhash3_directoryToMap,
);

export const xxhash3_128 = xxHashVariant<UInt64, bigint>(
  addon.xxhash3_128_oneshot,
  addon.xxhash3_128_createState,
  addon.xxhash3_128_file,
  addon.xxhash3_128_directory,
  addon.xxhash3_128_directoryToMap,
);

export default { xxhash32, xxhash64, xxhash3, xxhash3_128 };
