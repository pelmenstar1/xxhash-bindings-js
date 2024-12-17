import { test, expect } from 'vitest';
import { FileHashingType, xxhash32, xxhash64, xxhash3, xxhash3_128 } from '..';
import fs from 'fs';

const TEST_FILE_PATH = './test_data/image1.png';
const hashTypes = [FileHashingType.MAP, FileHashingType.BLOCK];

test.each([
  [xxhash32.file, 52811677],
  [xxhash64.file, BigInt('18153045472420481988')],
  [xxhash3.file, BigInt('12531405323377630900')],
  [xxhash3_128.file, BigInt('193898327962634967863812790837365759668')],
])('no seed', (hasher, expected) => {
  for (const type of hashTypes) {
    for (const options of [
      { path: TEST_FILE_PATH, seed: 0, type },
      { path: TEST_FILE_PATH, type },
      { path: TEST_FILE_PATH, seed: undefined, type },
    ]) {
      expect(hasher(options)).toBe(expected);
    }
  }
});

test.each([
  [xxhash32.file, 0, 0, 46947589],
  [xxhash32.file, 10_000_000_000, 10, 46947589],
  [xxhash32.file, 0, 100, 3233835924],
  [xxhash32.file, 100, 200, 3105022159],
  [xxhash32.file, 0, 0, 46947589],
  [xxhash32.file, 0, 10_000_000_000, 52811677],

  [xxhash64.file, 0, 0, BigInt('17241709254077376921')],
  [xxhash64.file, 10_000_000_000, 10, BigInt('17241709254077376921')],
  [xxhash64.file, 0, 100, BigInt('3640988105277625960')],
  [xxhash64.file, 100, 200, BigInt('11535842229069022349')],
  [xxhash64.file, 0, 0, BigInt('17241709254077376921')],
  [xxhash64.file, 0, 10_000_000_000, BigInt('18153045472420481988')],

  [xxhash3.file, 0, 0, BigInt('3244421341483603138')],
  [xxhash3.file, 10_000_000_000, 10, BigInt('3244421341483603138')],
  [xxhash3.file, 0, 100, BigInt('13321341031520568306')],
  [xxhash3.file, 100, 200, BigInt('15833511632417574391')],
  [xxhash3.file, 0, 0, BigInt('3244421341483603138')],
  [xxhash3.file, 0, 10_000_000_000, BigInt('12531405323377630900')],

  [xxhash3_128.file, 0, 0, BigInt('204254712233039002205064565430793619839')],
  [
    xxhash3_128.file,
    10_000_000_000,
    10,
    BigInt('204254712233039002205064565430793619839'),
  ],
  [xxhash3_128.file, 0, 100, BigInt('199877537612181396746579086691350335749')],
  [
    xxhash3_128.file,
    100,
    200,
    BigInt('332463148870137050248467284077638561955'),
  ],
  [xxhash3_128.file, 0, 0, BigInt('204254712233039002205064565430793619839')],
  [
    xxhash3_128.file,
    0,
    10_000_000_000,
    BigInt('193898327962634967863812790837365759668'),
  ],
])('file part no seed', (hasher, offset, length, expected) => {
  for (const type of hashTypes) {
    const baseOptions = {
      path: TEST_FILE_PATH,
      type,
      offset,
      length,
    };

    for (const options of [
      baseOptions,
      { ...baseOptions, seed: 0 },
      { ...baseOptions, seed: undefined },
    ]) {
      const actual = hasher(options);

      expect(
        actual,
        `type: ${type}; offset: ${offset}; length: ${length}`,
      ).toBe(expected);
    }
  }
});

test.each([
  [xxhash32.file, 1945663033],
  [xxhash64.file, BigInt('17740802669433987345')],
  [xxhash3.file, BigInt('8310716519890529791')],
  [xxhash3_128.file, BigInt('132161492315031615344357334049880780287')],
])('with seed', (hasher, expected) => {
  for (const type of hashTypes) {
    expect(hasher({ path: TEST_FILE_PATH, seed: 1, type })).toBe(expected);
  }
});

test.each([
  [xxhash32.file, 46947589],
  [xxhash64.file, BigInt('17241709254077376921')],
  [xxhash3.file, BigInt('3244421341483603138')],
  [xxhash3_128.file, BigInt('204254712233039002205064565430793619839')],
])('empty file no seed', (hasher, expected) => {
  for (const type of hashTypes) {
    expect(hasher({ path: './test_data/emptyfile', type })).toBe(expected);
  }
});

test.each([
  [xxhash32.file, 3068971186],
  [xxhash64.file, BigInt('13237225503670494420')],
  [xxhash3.file, BigInt('7335560060985733464')],
  [xxhash3_128.file, BigInt('296734076633237196744344171427223105880')],
])('one byte file no seed', (hasher, expected) => {
  for (const type of hashTypes) {
    expect(hasher({ path: './test_data/onebyte', type })).toBe(expected);
  }
});

test.runIf(fs.existsSync('/dev/zero')).each([
  [xxhash32.file, 593485017],
  [xxhash64.file, BigInt('8040990501003754519')],
  [xxhash3.file, BigInt('665452966430363425')],
  [xxhash3_128.file, BigInt('281489807592584962896215940306331225889')],
])('/dev/zero test', (hasher, expected) => {
  expect(
    hasher({ path: '/dev/zero', type: FileHashingType.BLOCK, length: 128 }),
  ).toBe(expected);
});

const hashers = [
  [xxhash32.file],
  [xxhash64.file],
  [xxhash3.file],
  [xxhash3_128.file],
] as const;

test.each(hashers)('throws on invalid path', (hasher) => {
  expect(() => hasher({ path: 123 as unknown as string })).toThrowError();
});

test.each(hashers)('throws on invalid seed', (hasher) => {
  expect(() =>
    hasher({ path: TEST_FILE_PATH, seed: '1' as unknown as number }),
  ).toThrowError();
});

test.each(hashers)('throws on invalid type', (hasher) => {
  expect(() =>
    hasher({ path: TEST_FILE_PATH, seed: 1, type: 100 as FileHashingType }),
  ).toThrowError();
});

test.each(hashers)('throws on non-existent path', (hasher) => {
  expect(() => hasher({ path: './.should_not_exist' })).toThrowError();
});
