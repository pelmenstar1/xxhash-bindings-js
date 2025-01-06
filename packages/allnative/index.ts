import { createRequire } from 'module';
import type {
  AsyncDirectoryHashOptions,
  BaseAsyncDirectoryHashOptions,
  FileHashOptions,
  UInt64,
  XxHashVariant,
} from 'xxhash-bindings-types';

export type {
  FileHashOptions,
  BaseDirectoryHashOptions,
  BaseSyncDirectoryHashOptions,
  BaseAsyncDirectoryHashOptions,
  AsyncDirectoryHashOptions,
  SyncDirectoryHashOptions,
  XxHashState,
  XxHashVariant,
  XxVariantName,
} from 'xxhash-bindings-types';

let addon: any;
const require = createRequire(import.meta.url);

try {
  addon = require('./build/Release/xxhash-allnative.node');
} catch {
  addon = require('./build/Debug/xxhash-allnative.node');
}

type AsyncBase<Arg, Result> = (
  arg: Arg,
  callback: (error: Error | undefined, value: Result | undefined) => void,
) => void;

type FileAsyncBase<S, H extends UInt64> = AsyncBase<FileHashOptions<S>, H>;

type DirectoryAsyncBase<S, H extends UInt64> = AsyncBase<
  AsyncDirectoryHashOptions<S, H>,
  undefined
>;

type DirectoryToMapAsyncBase<S, H extends UInt64> = AsyncBase<
  BaseAsyncDirectoryHashOptions<S>,
  Map<string, H>
>;

function xxHashVariant<S, H extends UInt64>(name: string): XxHashVariant<S, H> {
  const fileAsyncBase: FileAsyncBase<S, H> = addon[`${name}_fileAsync`];
  const directoryAsyncBase: DirectoryAsyncBase<S, H> =
    addon[`${name}_directoryAsync`];
  const directoryToMapAsyncBase: DirectoryToMapAsyncBase<S, H> =
    addon[`${name}_directoryToMapAsync`];

  return {
    oneshot: addon[`${name}_oneshot`],
    createState: addon[`${name}_createState`],
    file: addon[`${name}_file`],
    fileAsync: (options) => {
      return new Promise((resolve, reject) => {
        fileAsyncBase(options, (error, value) => {
          if (error === undefined) {
            resolve(value);
          } else {
            reject(error);
          }
        });
      });
    },
    directory: addon[`${name}_directory`],
    directoryAsync: (options) => {
      return new Promise((resolve, reject) => {
        directoryAsyncBase(options, (error) => {
          if (error == undefined) {
            resolve();
          } else {
            reject(error);
          }
        });
      });
    },
    directoryToMap: addon[`${name}_directoryToMap`],
    directoryToMapAsync: (options) => {
      return new Promise((resolve, reject) => {
        directoryToMapAsyncBase(options, (error, result) => {
          if (error === undefined) {
            resolve(result);
          } else {
            reject(error);
          }
        });
      });
    },
  };
}

export const xxhash32 = xxHashVariant<number, number>('xxhash32');
export const xxhash64 = xxHashVariant<UInt64, bigint>('xxhash64');
export const xxhash3 = xxHashVariant<UInt64, bigint>('xxhash3');
export const xxhash3_128 = xxHashVariant<UInt64, bigint>('xxhash3_128');

export default { xxhash32, xxhash64, xxhash3, xxhash3_128 };
