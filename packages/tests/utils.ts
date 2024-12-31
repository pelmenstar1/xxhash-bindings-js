import xxhashMin, { XxVariantName } from 'xxhash-bindings-min';
import xxhashAll from 'xxhash-bindings-allnative';

import path from 'path';

export type Lib = typeof xxhashMin;
export type VariantName = XxVariantName;

export const libs = [xxhashMin, xxhashAll];
export const variantNames = [
  'xxhash32',
  'xxhash64',
  'xxhash3',
  'xxhash3_128',
] as const;

export const TEST_DATA_PATH = './test_data';

export function testData(...paths: string[]): string {
  return path.join(TEST_DATA_PATH, ...paths);
}
