import xxhashMin from 'xxhash-bindings-min';
import xxhashAll from 'xxhash-bindings-allnative';

export type AnyArray<Arr extends unknown[]> = {
  [Index in keyof Arr]: unknown;
};

export function hideArgumentTypes<Args extends unknown[], R>(
  func: (...args: Args) => R,
): (...args: AnyArray<Args>) => R {
  return (...args) => func(...(args as Args));
}

export type Lib = typeof xxhashMin;
export type VariantName = keyof Lib;

export const libs = [xxhashMin, xxhashAll];
export const variantNames = [
  'xxhash32',
  'xxhash64',
  'xxhash3',
  'xxhash3_128',
] as const;
