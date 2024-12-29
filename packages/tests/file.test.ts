import { test, expect } from 'vitest';
import fs from 'fs';
import { VariantName, libs } from './utils';

const TEST_FILE_PATH = './test_data/image1.png';

const preferMapValues = [undefined, false, true];

test.each<[VariantName, number | bigint]>([
  ['xxhash32', 52811677],
  ['xxhash64', BigInt('18153045472420481988')],
  ['xxhash3', BigInt('12531405323377630900')],
  ['xxhash3_128', BigInt('193898327962634967863812790837365759668')],
])('no seed', (name, expected) => {
  for (const lib of libs) {
    const { file } = lib[name];

    for (const preferMap of preferMapValues) {
      const baseOptions = { path: TEST_FILE_PATH, preferMap };

      for (const options of [
        baseOptions,
        { ...baseOptions, seed: 0 },
        { ...baseOptions, seed: undefined },
      ]) {
        expect(file(options)).toBe(expected);
      }
    }
  }
});

test.each<[VariantName, number, number, number | bigint]>([
  ['xxhash32', 0, 0, 46947589],
  ['xxhash32', 10_000_000_000, 10, 46947589],
  ['xxhash32', 0, 100, 3233835924],
  ['xxhash32', 100, 200, 3105022159],
  ['xxhash32', 0, 0, 46947589],
  ['xxhash32', 0, 10_000_000_000, 52811677],

  ['xxhash64', 0, 0, BigInt('17241709254077376921')],
  ['xxhash64', 10_000_000_000, 10, BigInt('17241709254077376921')],
  ['xxhash64', 0, 100, BigInt('3640988105277625960')],
  ['xxhash64', 100, 200, BigInt('11535842229069022349')],
  ['xxhash64', 0, 0, BigInt('17241709254077376921')],
  ['xxhash64', 0, 10_000_000_000, BigInt('18153045472420481988')],

  ['xxhash3', 0, 0, BigInt('3244421341483603138')],
  ['xxhash3', 10_000_000_000, 10, BigInt('3244421341483603138')],
  ['xxhash3', 0, 100, BigInt('13321341031520568306')],
  ['xxhash3', 100, 200, BigInt('15833511632417574391')],
  ['xxhash3', 0, 0, BigInt('3244421341483603138')],
  ['xxhash3', 0, 10_000_000_000, BigInt('12531405323377630900')],

  ['xxhash3_128', 0, 0, BigInt('204254712233039002205064565430793619839')],
  [
    'xxhash3_128',
    10_000_000_000,
    10,
    BigInt('204254712233039002205064565430793619839'),
  ],
  ['xxhash3_128', 0, 100, BigInt('199877537612181396746579086691350335749')],
  ['xxhash3_128', 100, 200, BigInt('332463148870137050248467284077638561955')],
  ['xxhash3_128', 0, 0, BigInt('204254712233039002205064565430793619839')],
  [
    'xxhash3_128',
    0,
    10_000_000_000,
    BigInt('193898327962634967863812790837365759668'),
  ],
])('file part no seed', (name, offset, length, expected) => {
  for (const lib of libs) {
    const { file } = lib[name];
    for (const preferMap of preferMapValues) {
      const baseOptions = {
        path: TEST_FILE_PATH,
        preferMap,
        offset,
        length,
      };

      for (const options of [
        baseOptions,
        { ...baseOptions, seed: 0 },
        { ...baseOptions, seed: undefined },
      ]) {
        const actual = file(options);

        expect(actual).toBe(expected);
      }
    }
  }
});

test.each<[VariantName, number | bigint]>([
  ['xxhash32', 1945663033],
  ['xxhash64', BigInt('17740802669433987345')],
  ['xxhash3', BigInt('8310716519890529791')],
  ['xxhash3_128', BigInt('132161492315031615344357334049880780287')],
])('with seed', (name, expected) => {
  for (const lib of libs) {
    const { file } = lib[name];

    for (const preferMap of preferMapValues) {
      expect(file({ path: TEST_FILE_PATH, seed: 1, preferMap })).toBe(expected);
    }
  }
});

test.each([
  ['xxhash32', 46947589],
  ['xxhash64', BigInt('17241709254077376921')],
  ['xxhash3', BigInt('3244421341483603138')],
  ['xxhash3_128', BigInt('204254712233039002205064565430793619839')],
])('empty file no seed', (name, expected) => {
  for (const lib of libs) {
    const { file } = lib[name];

    for (const preferMap of preferMapValues) {
      expect(file({ path: './test_data/emptyfile', preferMap })).toBe(expected);
    }
  }
});

test.each([
  ['xxhash32', 3068971186],
  ['xxhash64', BigInt('13237225503670494420')],
  ['xxhash3', BigInt('7335560060985733464')],
  ['xxhash3_128', BigInt('296734076633237196744344171427223105880')],
])('one byte file no seed', (name, expected) => {
  for (const lib of libs) {
    const { file } = lib[name];
    for (const preferMap of preferMapValues) {
      expect(file({ path: './test_data/onebyte', preferMap })).toBe(expected);
    }
  }
});

test.runIf(fs.existsSync('/dev/zero')).each([
  ['xxhash32', 593485017],
  ['xxhash64', BigInt('8040990501003754519')],
  ['xxhash3', BigInt('665452966430363425')],
  ['xxhash3_128', BigInt('281489807592584962896215940306331225889')],
])('/dev/zero test', (name, expected) => {
  for (const lib of libs) {
    const { file } = lib[name];
    for (const preferMap of preferMapValues) {
      expect(file({ path: '/dev/zero', preferMap, length: 128 })).toBe(
        expected,
      );
    }
  }
});

const hashers: [VariantName][] = [
  ['xxhash32'],
  ['xxhash64'],
  ['xxhash3'],
  ['xxhash3_128'],
] as const;

test.each(hashers)('throws on invalid path', (name) => {
  for (const lib of libs) {
    const { file } = lib[name];
    expect(() => file({ path: 123 as unknown as string })).toThrowError();
  }
});

test.each<[VariantName, string]>([
  ['xxhash32', 'number or undefined'],
  ['xxhash64', 'number, bigint or undefined'],
  ['xxhash3', 'number, bigint or undefined'],
  ['xxhash3_128', 'number, bigint or undefined'],
])('throws on invalid seed', (name, expected) => {
  for (const lib of libs) {
    const { file } = lib[name];

    expect(() =>
      file({ path: TEST_FILE_PATH, seed: '1' as unknown as number }),
    ).toThrowError(
      Error(`Expected type of the property "seed" is ${expected}`),
    );
  }
});

test.each(hashers)('throws on invalid preferMap', (name) => {
  for (const lib of libs) {
    const { file } = lib[name];

    expect(() =>
      file({
        path: TEST_FILE_PATH,
        seed: 1,
        preferMap: 100 as unknown as boolean,
      }),
    ).toThrowError(
      Error(
        'Expected type of the property "preferMap" is boolean or undefined',
      ),
    );
  }
});

test.each(hashers)('throws on non-existent path', (name) => {
  for (const lib of libs) {
    const { file } = lib[name];
    expect(() => file({ path: './.should_not_exist' })).toThrowError();
  }
});
