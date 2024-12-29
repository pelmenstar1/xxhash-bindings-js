import { beforeAll, expect, test } from 'vitest';
import { vi } from 'vitest';
import { promises as fs, ObjectEncodingOptions } from 'fs';
import { VariantName, Lib, libs, variantNames } from './utils';
import { fail } from 'assert';
import process from 'process';

const BASE_DIR = './test_data';
const PATH_SUFFIXES =
  process.platform == 'win32' ? ['', '/', '//', '\\', '\\\\'] : ['', '/', '//'];

function forEachOptions<N extends VariantName>(
  name: N,
  dirName: string,
  block: (options: {
    lib: Lib;
    directory: Lib[N]['directory'];
    preferMap: boolean | undefined;
    path: string;
  }) => void,
): void {
  for (const lib of libs) {
    const { directory } = lib[name];

    for (const preferMap of [undefined, false, true]) {
      for (const suffix of PATH_SUFFIXES) {
        const path = `${BASE_DIR}/${dirName}${suffix}`;

        block({ lib, directory, preferMap, path });
      }
    }
  }
}

beforeAll(async () => {
  await Promise.all([
    fs.mkdir(`${BASE_DIR}/dir/`, { recursive: true }),
    fs.mkdir(`${BASE_DIR}/empty_dir/`, { recursive: true }),
    fs.mkdir(`${BASE_DIR}/dir/empty_dir`, { recursive: true }),
    fs.mkdir(`${BASE_DIR}/dir/dir2/`, { recursive: true }),
  ]);

  const options: ObjectEncodingOptions = { encoding: 'utf8' };

  await Promise.all([
    fs.writeFile(`${BASE_DIR}/dir/file1.txt`, 'content', options),
    fs.writeFile(`${BASE_DIR}/dir/file2.txt`, 'content 2', options),
    fs.writeFile(`${BASE_DIR}/dir/dir2/file3.txt`, 'content 3', options),
  ]);
});

test.each<[VariantName]>([
  ['xxhash32'],
  ['xxhash64'],
  ['xxhash3'],
  ['xxhash3_128'],
])('empty dir', (name) => {
  forEachOptions(name, 'empty_dir', ({ directory, preferMap, path }) => {
    for (const seed of [undefined, 0, 1]) {
      const actual = directory({
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
  forEachOptions(name, 'dir', ({ lib, directory, preferMap, path }) => {
    const baseOptions = { path, preferMap };

    for (const options of [
      baseOptions,
      { ...baseOptions, seed: 0 },
      { ...baseOptions, seed: undefined },
    ]) {
      const actual = directory(options);
      const expectedMap = new Map(Object.entries(expected));

      for (const [fileName, value] of expectedMap) {
        const fileExpected = lib[name].file({
          ...options,
          path: `${BASE_DIR}/dir/${fileName}`,
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
  forEachOptions(name, 'dir', ({ lib, directory, preferMap, path }) => {
    const options = {
      path,
      preferMap,
      seed: 1,
    };

    const actual = directory(options);
    const expectedMap = new Map(Object.entries(expected));

    for (const [fileName, value] of expectedMap) {
      const fileExpected = lib[name].file({
        ...options,
        path: `${BASE_DIR}/dir/${fileName}`,
      });

      expect(value).toBe(fileExpected);
    }

    expect(actual).toEqual(expectedMap);
  });
});

function createExpectedMapFromNames(
  file: Lib[keyof Lib]['file'],
  names: string[],
): Map<string, number | bigint> {
  const result = new Map<string, number | bigint>();
  for (const fileName of names) {
    const value = file({
      path: `${BASE_DIR}/dir/${fileName}`,
      seed: 1,
    });

    result.set(fileName, value);
  }

  return result;
}

test.each(variantNames.map((name) => [name]))(
  'dir with seed acceptFile',
  (name) => {
    forEachOptions(name, 'dir', ({ lib, directory, preferMap, path }) => {
      const acceptFile = vi.fn((fileName: string): boolean => {
        if (fileName !== 'file1.txt' && fileName !== 'file2.txt') {
          fail(`acceptFile called with ("${fileName}")`);
        }

        return fileName === 'file1.txt';
      });

      const actual = directory({
        seed: 1,
        path,
        preferMap,
        acceptFile,
      });

      const expectedMap = createExpectedMapFromNames(lib[name].file, [
        'file1.txt',
      ]);

      expect(actual).toEqual(expectedMap);
      expect(acceptFile).toBeCalledTimes(2);
    });
  },
);

test.each(variantNames.map((name) => [name]))(
  'dir with seed acceptFile returns non-boolean',
  (name) => {
    forEachOptions(name, 'dir', ({ lib, directory, preferMap, path }) => {
      const acceptFile = vi.fn((fileName: string) =>
        fileName === 'file1.txt' ? ('true' as unknown as boolean) : undefined,
      );

      const actual = directory({
        seed: 1,
        path,
        preferMap,
        acceptFile,
      });

      const expectedMap = createExpectedMapFromNames(lib[name].file, [
        'file1.txt',
      ]);

      expect(actual).toEqual(expectedMap);
      expect(acceptFile).toBeCalledTimes(2);
    });
  },
);

test.each(variantNames.map((name) => [name]))(
  'dir acceptFile throwing',
  (name) => {
    for (const lib of libs) {
      const { directory } = lib[name];

      const error = new Error('custom error message');
      const acceptFile = () => {
        throw error;
      };

      expect(() =>
        directory({
          path: `${BASE_DIR}/dir/`,
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
      `${BASE_DIR}/__does_not_exist`,
      `${BASE_DIR}/emptyfile`,
    ];

    if (process.platform !== 'win32') {
      invalidPaths.push(`${BASE_DIR}\\`);
    }

    for (const lib of libs) {
      const { directory } = lib[name];

      for (const preferMap of [undefined, false, true]) {
        for (const path of invalidPaths) {
          expect(() => directory({ path, preferMap })).toThrowError();
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
    const { directory } = lib[name];

    expect(() =>
      directory({ path: `${BASE_DIR}/dir`, seed: '1' as unknown as number }),
    ).toThrowError(
      Error(`Expected type of the property "seed" is ${expected}`),
    );
  }
});

test.each(variantNames.map((name) => [name]))(
  'throws on invalid preferMap',
  (name) => {
    for (const lib of libs) {
      const { directory } = lib[name];

      expect(() =>
        directory({
          path: `${BASE_DIR}/dir`,
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
      const { directory } = lib[name];

      expect(() =>
        directory({
          path: `${BASE_DIR}/dir`,
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
