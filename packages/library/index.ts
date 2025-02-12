import { createRequire } from 'module';

type UInt64 = number | bigint;
type MaybePromise<T> = T | Promise<T>;

export type XxVariantName = 'xxhash32' | 'xxhash64' | 'xxhash3' | 'xxhash3_128';

export type AcceptFile = (name: string) => boolean;
export type AcceptFileAsync = (name: string) => MaybePromise<boolean>;

export type OnFile<H> = (name: string, hash: H) => void;
export type OnFileAsync<H> = (name: string, hash: H) => MaybePromise<void>;

export type FileHashOptions<S> = {
  path: string;
  seed?: S;
  offset?: UInt64;
  length?: UInt64;
  preferMap?: boolean;
};

export interface BaseDirectoryHashOptions<S> {
  path: string;
  seed?: S;
}

export interface BaseSyncDirectoryHashOptions<S>
  extends BaseDirectoryHashOptions<S> {
  preferMap?: boolean;
  acceptFile?: AcceptFile;
}

export interface BaseAsyncDirectoryHashOptions<S>
  extends BaseDirectoryHashOptions<S> {
  acceptFile?: AcceptFile;
}

export interface SyncDirectoryHashOptions<S, H>
  extends BaseSyncDirectoryHashOptions<S> {
  onFile: OnFile<H>;
}

export interface AsyncDirectoryHashOptions<S, H>
  extends BaseAsyncDirectoryHashOptions<S> {
  onFile: OnFile<H>;
}

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

  directory(options: SyncDirectoryHashOptions<S, H>): void;
  directoryAsync(options: AsyncDirectoryHashOptions<S, H>): Promise<void>;

  directoryToMap(options: BaseSyncDirectoryHashOptions<S>): Map<string, H>;
  directoryToMapAsync(
    options: BaseAsyncDirectoryHashOptions<S>,
  ): Promise<Map<string, H>>;
};

const require = createRequire(import.meta.url);
const addon = require(`./xxhash-${process.platform}-${process.arch}.node`);

type AsyncCallback<T> = (
  error: Error | undefined,
  value: T | undefined,
) => void;

type AsyncBase<T, R> = (arg: T, callback: AsyncCallback<R>) => void;

type FileAsyncBase<S, H extends UInt64> = AsyncBase<FileHashOptions<S>, H>;

type DirectoryAsyncBase<S, H extends UInt64> = AsyncBase<
  AsyncDirectoryHashOptions<S, H>,
  undefined
>;

type DirectoryToMapAsyncBase<S, H extends UInt64> = AsyncBase<
  BaseAsyncDirectoryHashOptions<S>,
  Map<string, H>
>;

function toPromise<T, R>(
  func: (options: T, callback: AsyncCallback<R>) => void,
): (options: T) => Promise<R> {
  return (options) => {
    return new Promise((resolve, reject) => {
      func(options, (error, value) => {
        if (error === undefined) {
          resolve(value);
        } else {
          reject(error);
        }
      });
    });
  };
}

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
    directory: addon[`${name}_directory`],
    directoryToMap: addon[`${name}_directoryToMap`],

    fileAsync: toPromise(fileAsyncBase),
    directoryAsync: toPromise(directoryAsyncBase),
    directoryToMapAsync: toPromise(directoryToMapAsyncBase),
  };
}

export const xxhash32 = xxHashVariant<number, number>('xxhash32');
export const xxhash64 = xxHashVariant<UInt64, bigint>('xxhash64');
export const xxhash3 = xxHashVariant<UInt64, bigint>('xxhash3');
export const xxhash3_128 = xxHashVariant<UInt64, bigint>('xxhash3_128');

export default { xxhash32, xxhash64, xxhash3, xxhash3_128 };
