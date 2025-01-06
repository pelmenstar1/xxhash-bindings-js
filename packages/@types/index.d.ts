export type UInt64 = number | bigint;
export type XxVariantName = 'xxhash32' | 'xxhash64' | 'xxhash3' | 'xxhash3_128';

type MaybePromise<T> = T | Promise<T>;

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
