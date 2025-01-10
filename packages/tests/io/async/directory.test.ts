import { expect, test } from 'vitest';
import { libs, testData, variantNames } from '@/utils';
import { setupTests } from '../base/directory';
import { expectToThrowAsyncFactory } from '../base/helpers';

setupTests({
  getDirectoryFactory: (lib, name) => {
    return async (options) => {
      const directoryAsync = lib[name].directoryAsync;

      const resultMap = new Map<string, number | bigint>();

      await directoryAsync({
        ...options,
        onFile: (name: string, result: number | bigint) => {
          resultMap.set(name, result);
        },
      });

      return resultMap as Map<string, number>;
    };
  },
  expectToThrowError: expectToThrowAsyncFactory(),
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
