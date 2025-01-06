import {
  Lib,
  libs,
  TEST_DATA_PATH,
  testData,
  VariantName,
  variantNames,
} from '@/utils';
import { expect, test, vi } from 'vitest';
import { fail } from 'assert';
import { PATH_SUFFIXES } from '../directoryUtils';

type DirectoryToMapFactory = (
  lib: Lib,
  name: VariantName,
) => Lib[VariantName]['directoryToMap'];

export { setupDirectories } from '../directoryUtils';

export function setupTests(directoryToMapFactory: DirectoryToMapFactory) {
  function forEachOptions(
    name: VariantName,
    dirName: string,
    block: (options: {
      lib: Lib;
      directoryToMap: Lib[VariantName]['directoryToMap'];
      preferMap: boolean | undefined;
      path: string;
    }) => void,
  ): void {
    for (const lib of libs) {
      const directoryToMap = directoryToMapFactory(lib, name);

      for (const preferMap of [undefined, false, true]) {
        for (const suffix of PATH_SUFFIXES) {
          const path = testData(`${dirName}${suffix}`);

          block({ lib, directoryToMap, preferMap, path });
        }
      }
    }
  }

  test.each(variantNames.map((name) => [name]))('empty dir', (name) => {
    forEachOptions(name, 'empty_dir', ({ directoryToMap, preferMap, path }) => {
      for (const seed of [undefined, 0, 1]) {
        const actual = directoryToMap({
          path,
          seed,
          preferMap,
        });

        expect(actual).toEqual(new Map());
      }
    });
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
    forEachOptions(name, 'dir', ({ lib, directoryToMap, preferMap, path }) => {
      const baseOptions = { path, preferMap };

      for (const options of [
        baseOptions,
        { ...baseOptions, seed: 0 },
        { ...baseOptions, seed: undefined },
      ]) {
        const actual = directoryToMap(options);
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
    });
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
    forEachOptions(name, 'dir', ({ lib, directoryToMap, preferMap, path }) => {
      const options = {
        path,
        preferMap,
        seed: 1,
      };

      const actual = directoryToMap(options);
      const expectedMap = new Map(Object.entries(expected));

      for (const [fileName, value] of expectedMap) {
        const fileExpected = lib[name].file({
          ...options,
          path: testData('dir', fileName),
        });

        expect(value).toBe(fileExpected);
      }

      expect(actual).toEqual(expectedMap);
    });
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
      forEachOptions(
        name,
        'dir',
        ({ lib, directoryToMap, preferMap, path }) => {
          const acceptFile = vi.fn((fileName: string): boolean => {
            if (fileName !== 'file1.txt' && fileName !== 'file2.txt') {
              fail(`acceptFile called with ("${fileName}")`);
            }

            return fileName === 'file1.txt';
          });

          const actual = directoryToMap({
            seed: 1,
            path,
            preferMap,
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
      forEachOptions(
        name,
        'dir',
        ({ lib, directoryToMap, preferMap, path }) => {
          const acceptFile = vi.fn((fileName: string) =>
            fileName === 'file1.txt'
              ? ('true' as unknown as boolean)
              : undefined,
          );

          const actual = directoryToMap({
            seed: 1,
            path,
            preferMap,
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
    (name) => {
      for (const lib of libs) {
        const directoryToMap = directoryToMapFactory(lib, name);

        const error = new Error('custom error message');
        const acceptFile = () => {
          throw error;
        };

        expect(() =>
          directoryToMap({
            path: `${TEST_DATA_PATH}/dir/`,
            acceptFile,
          }),
        ).toThrowError(error);
      }
    },
  );

  test.each(variantNames.map((name) => [name]))(
    'throws on invalid path',
    (name) => {
      const invalidPaths = [
        `${TEST_DATA_PATH}/__does_not_exist`,
        `${TEST_DATA_PATH}/emptyfile`,
      ];

      if (process.platform !== 'win32') {
        invalidPaths.push(`${TEST_DATA_PATH}\\`);
      }

      for (const lib of libs) {
        const directoryToMap = directoryToMapFactory(lib, name);

        for (const preferMap of [undefined, false, true]) {
          for (const path of invalidPaths) {
            expect(() => directoryToMap({ path, preferMap })).toThrowError();
          }
        }
      }
    },
  );

  test.each<[VariantName, string]>([
    ['xxhash32', 'number or undefined'],
    ['xxhash64', 'number, bigint or undefined'],
    ['xxhash3', 'number, bigint or undefined'],
    ['xxhash3_128', 'number, bigint or undefined'],
  ])('throws on invalid seed', (name, expected) => {
    for (const lib of libs) {
      const directoryToMap = directoryToMapFactory(lib, name);

      expect(() =>
        directoryToMap({
          path: `${TEST_DATA_PATH}/dir`,
          seed: '1' as unknown as number,
        }),
      ).toThrowError(
        Error(`Expected type of the property "seed" is ${expected}`),
      );
    }
  });

  test.each(variantNames.map((name) => [name]))(
    'throws on invalid preferMap',
    (name) => {
      for (const lib of libs) {
        const directoryToMap = directoryToMapFactory(lib, name);

        expect(() =>
          directoryToMap({
            path: `${TEST_DATA_PATH}/dir`,
            seed: 1,
            preferMap: 100 as unknown as boolean,
          }),
        ).toThrowError(
          Error(
            'Expected type of the property "preferMap" is boolean or undefined',
          ),
        );
      }
    },
  );

  test.each(variantNames.map((name) => [name]))(
    'throws on invalid acceptFile',
    (name) => {
      for (const lib of libs) {
        const directoryToMap = directoryToMapFactory(lib, name);

        expect(() =>
          directoryToMap({
            path: `${TEST_DATA_PATH}/dir`,
            seed: 1,
            preferMap: true,
            acceptFile: 0 as unknown as () => boolean,
          }),
        ).toThrowError(
          Error(
            'Expected type of the property "acceptFile" is function or undefined',
          ),
        );
      }
    },
  );
}
