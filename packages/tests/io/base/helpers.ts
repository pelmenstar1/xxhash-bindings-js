import { expect } from 'vitest';

type ExpectToThrow<T> = (
  options: T,
  hasher: (arg: T) => unknown,
  error?: Error,
) => Promise<void>;

export function expectToThrowSyncFactory<T>(): ExpectToThrow<T> {
  return (options, hasher, error) => {
    expect(() => hasher(options)).toThrowError(error);

    return Promise.resolve();
  };
}

export function expectToThrowAsyncFactory<T>(): ExpectToThrow<T> {
  return async (options, hasher, error) => {
    const builder = expect(() => hasher(options)).rejects;

    return error === undefined
      ? await builder.toBeTruthy()
      : await builder.toEqual(error);
  };
}

type UnknownFunction = (...args: unknown[]) => unknown;
type AsyncFunction<F> = F extends (...args: infer Args) => infer R
  ? (...args: Args) => Promise<R>
  : never;

export function syncToAsync<F extends UnknownFunction>(
  fn: F,
): AsyncFunction<F> {
  return ((...args) => Promise.resolve(fn(...args))) as AsyncFunction<F>;
}

export function numberWithBigint(value: number): [number, bigint] {
  return [value, BigInt(value)];
}
