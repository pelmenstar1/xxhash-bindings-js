import path from 'path';

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
