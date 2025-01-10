import {
  Lib,
  TEST_DATA_PATH,
  testData,
  VariantName,
  variantNames,
} from '@/utils';
import { describe, expect, test, vi } from 'vitest';
import { fail } from 'assert';
import minimum, {
  BaseAsyncDirectoryHashOptions,
  BaseSyncDirectoryHashOptions,
  XxVariantName,
} from 'xxhash-bindings-min';
import allnative from 'xxhash-bindings-allnative';

type GenericDirectoryOptions =
  | BaseSyncDirectoryHashOptions<number>
  | BaseAsyncDirectoryHashOptions<number>;

type GenericDirectoryHasher = (
  options: GenericDirectoryOptions,
) => Promise<Map<string, number | bigint>>;

type SetupTestsContext = {
  getDirectoryFactory: (
    lib: Lib,
    variant: XxVariantName,
  ) => GenericDirectoryHasher;

  expectToThrowError(
    options: GenericDirectoryOptions,
    hasher: GenericDirectoryHasher,
    error?: Error,
  ): Promise<void>;
};

const PATH_SUFFIXES =
  process.platform == 'win32' ? ['', '/', '//', '\\', '\\\\'] : ['', '/', '//'];

export function setupTests(context: SetupTestsContext) {
  setupTestsOnLib(context, minimum, 'minimum');
  setupTestsOnLib(context, allnative, 'allnative');
}

export function setupTestsOnLib(
  { getDirectoryFactory, expectToThrowError }: SetupTestsContext,
  lib: Lib,
  libName: string,
) {
  async function forEachOptions(
    name: VariantName,
    dirName: string,
    block: (options: {
      directoryToMapAsync: GenericDirectoryHasher;
      path: string;
    }) => void | Promise<void>,
  ): Promise<void> {
    const directoryToMapAsync = getDirectoryFactory(lib, name);

    for (const suffix of PATH_SUFFIXES) {
      const path = testData(`${dirName}${suffix}`);

      await block({ directoryToMapAsync, path });
    }
  }

  describe(libName, () => {
    test.each(variantNames.map((name) => [name]))('empty dir', (name) => {
      return forEachOptions(
        name,
        'empty_dir',
        async ({ directoryToMapAsync, path }) => {
          for (const seed of [undefined, 0, 1]) {
            const actual = await directoryToMapAsync({
              path,
              seed,
            });

            expect(actual).toEqual(new Map());
          }
        },
      );
    });

    test.each<[VariantName, Record<string, number | bigint>]>([
      ['xxhash32', { 'file1.txt': 2709547657, 'file2.txt': 2400679296 }],
      [
        'xxhash64',
        {
          'file1.txt': BigInt('7807861979271768572'),
          'file2.txt': BigInt('6716447616866097093'),
        },
      ],
      [
        'xxhash3',
        {
          'file1.txt': BigInt('6193209363630369380'),
          'file2.txt': BigInt('7469767244817125416'),
        },
      ],
      [
        'xxhash3_128',
        {
          'file1.txt': BigInt('193392663078994241937916373705623688844'),
          'file2.txt': BigInt('282518126491299247794495067982602648274'),
        },
      ],
    ])('dir no seed', (name, expected) => {
      return forEachOptions(
        name,
        'dir',
        async ({ directoryToMapAsync, path }) => {
          const baseOptions = { path };

          for (const options of [
            baseOptions,
            { ...baseOptions, seed: 0 },
            { ...baseOptions, seed: undefined },
          ]) {
            const actual = await directoryToMapAsync(options);
            const expectedMap = new Map(Object.entries(expected));

            for (const [fileName, value] of expectedMap) {
              const fileExpected = lib[name].file({
                ...options,
                path: testData('dir', fileName),
              });

              expect(value).toBe(fileExpected);
            }

            expect(actual).toEqual(expectedMap);
          }
        },
      );
    });

    test.each<[VariantName, Record<string, number | bigint>]>([
      ['xxhash32', { 'file1.txt': 2323014111, 'file2.txt': 4219041358 }],
      [
        'xxhash64',
        {
          'file1.txt': BigInt('4476637912297980560'),
          'file2.txt': BigInt('5425576953216100815'),
        },
      ],
      [
        'xxhash3',
        {
          'file1.txt': BigInt('18413389910768766083'),
          'file2.txt': BigInt('8003765238400060217'),
        },
      ],
      [
        'xxhash3_128',
        {
          'file1.txt': BigInt('40839179373527625521031880340436746401'),
          'file2.txt': BigInt('142466360073840497451964377040934298692'),
        },
      ],
    ])('dir with seed', (name, expected) => {
      return forEachOptions(
        name,
        'dir',
        async ({ directoryToMapAsync, path }) => {
          const options = {
            path,
            seed: 1,
          };

          const actual = await directoryToMapAsync(options);
          const expectedMap = new Map(Object.entries(expected));

          for (const [fileName, value] of expectedMap) {
            const fileExpected = lib[name].file({
              ...options,
              path: testData('dir', fileName),
            });

            expect(value).toBe(fileExpected);
          }

          expect(actual).toEqual(expectedMap);
        },
      );
    });

    function createExpectedMapFromNames(
      file: Lib[keyof Lib]['file'],
    ): Map<string, number | bigint> {
      const result = new Map<string, number | bigint>();
      for (const fileName of ['file1.txt']) {
        const value = file({
          path: testData('dir', fileName),
          seed: 1,
        });

        result.set(fileName, value);
      }

      return result;
    }

    test.each(variantNames.map((name) => [name]))(
      'dir with seed acceptFile',
      (name) => {
        return forEachOptions(
          name,
          'dir',
          async ({ directoryToMapAsync, path }) => {
            const acceptFile = vi.fn((fileName: string): boolean => {
              if (fileName !== 'file1.txt' && fileName !== 'file2.txt') {
                fail(`acceptFile called with ("${fileName}")`);
              }

              return fileName === 'file1.txt';
            });

            const actual = await directoryToMapAsync({
              seed: 1,
              path,
              acceptFile,
            });

            const expectedMap = createExpectedMapFromNames(lib[name].file);

            expect(actual).toEqual(expectedMap);
            expect(acceptFile).toBeCalledTimes(2);
          },
        );
      },
    );

    test.each(variantNames.map((name) => [name]))(
      'dir with seed acceptFile returns non-boolean',
      (name) => {
        return forEachOptions(
          name,
          'dir',
          async ({ directoryToMapAsync, path }) => {
            const acceptFile = vi.fn((fileName: string) =>
              fileName === 'file1.txt'
                ? ('true' as unknown as boolean)
                : undefined,
            );

            const actual = await directoryToMapAsync({
              seed: 1,
              path,
              acceptFile,
            });

            const expectedMap = createExpectedMapFromNames(lib[name].file);

            expect(actual).toEqual(expectedMap);
            expect(acceptFile).toBeCalledTimes(2);
          },
        );
      },
    );

    test.each(variantNames.map((name) => [name]))(
      'dir acceptFile throwing',
      async (name) => {
        const directoryToMap = getDirectoryFactory(lib, name);

        const error = new Error('custom error message');
        const acceptFile = () => {
          throw error;
        };

        expectToThrowError(
          {
            path: `${TEST_DATA_PATH}/dir/`,
            acceptFile,
          },
          directoryToMap,
          error,
        );
      },
    );

    test.each(variantNames.map((name) => [name]))(
      'throws on invalid path',
      async (name) => {
        const invalidPaths = [
          `${TEST_DATA_PATH}/__does_not_exist`,
          `${TEST_DATA_PATH}/emptyfile`,
        ];

        if (process.platform !== 'win32') {
          invalidPaths.push(`${TEST_DATA_PATH}\\`);
        }

        const directoryToMapAsync = getDirectoryFactory(lib, name);

        for (const path of invalidPaths) {
          await expectToThrowError({ path }, directoryToMapAsync);
        }
      },
    );

    test.each<[VariantName, string]>([
      ['xxhash32', 'number or undefined'],
      ['xxhash64', 'number, bigint or undefined'],
      ['xxhash3', 'number, bigint or undefined'],
      ['xxhash3_128', 'number, bigint or undefined'],
    ])('throws on invalid seed', async (name, expected) => {
      const directoryToMap = getDirectoryFactory(lib, name);

      await expectToThrowError(
        { path: `${TEST_DATA_PATH}/dir`, seed: '1' as unknown as number },
        directoryToMap,
        Error(`Expected type of the property "seed" is ${expected}`),
      );
    });

    test.each(variantNames.map((name) => [name]))(
      'throws on invalid acceptFile',
      async (name) => {
        const directoryToMapAsync = getDirectoryFactory(lib, name);

        await expectToThrowError(
          {
            path: `${TEST_DATA_PATH}/dir`,
            seed: 1,
            acceptFile: 0 as unknown as () => boolean,
          },
          directoryToMapAsync,
          Error(
            'Expected type of the property "acceptFile" is function or undefined',
          ),
        );
      },
    );
  });
}
