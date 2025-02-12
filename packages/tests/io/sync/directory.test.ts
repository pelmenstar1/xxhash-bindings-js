import { expect, test } from 'vitest';
import { testData, variantNames } from '../../utils';
import { setupTests } from '../base/directory';
import { expectToThrowSyncFactory } from '../base/helpers';
import lib from 'xxhash-bindings';

setupTests({
  getDirectoryFactory: (name) => {
    return (options) => {
      const directory = lib[name].directory;

      const resultMap = new Map<string, number | bigint>();

      directory({
        ...options,
        onFile: (name: string, result: number | bigint) => {
          resultMap.set(name, result);
        },
      });

      return Promise.resolve(resultMap);
    };
  },
  expectToThrowError: expectToThrowSyncFactory(),
});

test.each(variantNames.map((name) => [name]))(
  'directory onFile throws',
  (name) => {
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
  },
);
