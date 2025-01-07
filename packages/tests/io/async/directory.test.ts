import { expect, test } from 'vitest';
import { setupTests } from './directoryTestUtils';
import type { BaseSyncDirectoryHashOptions } from 'xxhash-bindings-min';
import { libs, testData, variantNames } from '@/utils';

setupTests((lib, name) => {
  return async (options: BaseSyncDirectoryHashOptions<number | bigint>) => {
    const directoryAsync = lib[name].directoryAsync as (
      options: BaseSyncDirectoryHashOptions<number | bigint> & {
        onFile: (name: string, hash: number | bigint) => void;
      },
    ) => Promise<void>;

    const resultMap = new Map<string, number | bigint>();

    await directoryAsync({
      ...options,
      onFile: (name: string, result: number | bigint) => {
        resultMap.set(name, result);
      },
    });

    return resultMap as Map<string, number>;
  };
});

test.each(variantNames.map((name) => [name]))(
  'directory onFile throws',
  async (name) => {
    for (const lib of libs) {
      const { directoryAsync } = lib[name];
      const error = new Error('custom message');

      await expect(() =>
        directoryAsync({
          path: testData('dir'),
          onFile: () => {
            throw error;
          },
        }),
      ).rejects.toBeTruthy();
    }
  },
);
