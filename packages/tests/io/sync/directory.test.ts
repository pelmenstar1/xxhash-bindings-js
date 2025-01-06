import { beforeAll, expect, test } from 'vitest';
import { setupDirectories, setupTests } from './directoryTestUtils';
import type { BaseSyncDirectoryHashOptions } from 'xxhash-bindings-min';
import { libs, testData, variantNames } from '../../utils';

beforeAll(setupDirectories);
setupTests((lib, name) => {
  return (options: BaseSyncDirectoryHashOptions<number | bigint>) => {
    const directory = lib[name].directory as (
      options: BaseSyncDirectoryHashOptions<number | bigint> & {
        onFile: (name: string, hash: number | bigint) => void;
      },
    ) => void;

    const resultMap = new Map<string, number | bigint>();

    directory({
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
  (name) => {
    for (const lib of libs) {
      const { directory } = lib[name];
      const error = new Error('custom message');

      expect(() =>
        directory({
          path: testData('dir'),
          onFile: () => {
            throw error;
          },
        }),
      ).toThrowError(error);
    }
  },
);
