import { createRequire } from 'module';
import type { UInt64, XxHashVariant } from 'xxhash-bindings-types';

export type {
  DirectoryHashingOptions,
  FileHashingOptions,
  XxHashState,
  XxHashVariant,
  XxVariantName,
} from 'xxhash-bindings-types';

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
