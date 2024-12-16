import { describe, expect, test } from 'vitest';
import {
  xxhash32,
  xxhash64,
  xxhash3,
  xxhash3_128,
  FileHashingType,
  FileHashingOptions,
  XxHashState,
} from './index.js';

type AnyArray<Arr extends unknown[]> = {
  [Index in keyof Arr]: unknown;
};

function hideArgumentTypes<Args extends unknown[], R>(
  func: (...args: Args) => R,
  ...args: AnyArray<Args>
): R {
  return func(...(args as Args));
}

describe('oneshots', () => {
  const testData = Uint8Array.from([97, 98, 99, 100]);

  test('xxhash32', () => {
    expect(xxhash32.oneshot(testData, 0)).toBe(0xa3643705);
  });

  test('xxhash64', () => {
    expect(xxhash64.oneshot(testData, 1)).toBe(BigInt('0xf5dcbd6dee3c9553'));
  });

  test('xxhash3', () => {
    const expected = BigInt('7248448420886124688');

    expect(xxhash3.oneshot(testData)).toBe(expected);
    expect(xxhash3.oneshot(testData, 0)).toBe(expected);
    expect(xxhash3.oneshot(testData, undefined)).toBe(expected);
  });

  test('xxhash3 with seed', () => {
    expect(xxhash3.oneshot(testData, 1)).toBe(BigInt('13053297174767443700'));
  });

  test('xxhash3 (128)', () => {
    const expected = BigInt('187978674736816916650311503294403523901');

    expect(xxhash3_128.oneshot(testData)).toBe(expected);
    expect(xxhash3_128.oneshot(testData, 0)).toBe(expected);
    expect(xxhash3_128.oneshot(testData, undefined)).toBe(expected);
  });

  test('xxhash3 with seed (128)', () => {
    expect(xxhash3_128.oneshot(testData, 1)).toBe(
      BigInt('233827487414958832244112154636312345382'),
    );
  });

  function throwsTest<Args extends unknown[]>(
    testName: string,
    hasher: (...args: Args) => unknown,
    ...args: AnyArray<Args>
  ) {
    test(testName, () => {
      expect(() => hideArgumentTypes(hasher, ...args)).toThrow();
    });
  }

  function throwsInvalidTypeTest(
    functionName: string,
    hasher: (buffer: Uint8Array, seed: number) => unknown,
  ) {
    throwsTest(`${functionName} invalid buffer type`, hasher, undefined, 0);
    throwsTest(
      `${functionName} invalid seed type`,
      hasher,
      Uint8Array.of(0),
      '123',
    );
  }

  throwsInvalidTypeTest('xxhash32', xxhash32.oneshot);
  throwsInvalidTypeTest('xxhash64', xxhash64.oneshot);
  throwsInvalidTypeTest('xxhash3', xxhash3.oneshot);
  throwsInvalidTypeTest('xxhash3_128', xxhash3_128.oneshot);
});

describe('file hashing', () => {
  const TEST_FILE_PATH = './test_data/image1.png';

  function throwsTest<Args extends unknown[]>(
    testName: string,
    hasher: (...args: Args) => unknown,
    ...args: AnyArray<Args>
  ) {
    test(testName, () => {
      expect(() => hideArgumentTypes(hasher, ...args)).toThrow();
    });
  }

  function variantTests<T>(
    hasherName: string,
    hasher: (options: FileHashingOptions<number>) => T,
    seeds: [number, T][],
  ) {
    const types = [FileHashingType.MAP, FileHashingType.BLOCK];

    for (const type of types) {
      for (const [seed, expected] of seeds) {
        test(`${hasherName} (seed=${seed}, type=${type})`, () => {
          expect(hasher({ path: TEST_FILE_PATH, seed, type })).toBe(expected);

          if (seed == 0) {
            expect(hasher({ path: TEST_FILE_PATH, type })).toBe(expected);
            expect(
              hasher({ path: TEST_FILE_PATH, seed: undefined, type }),
            ).toBe(expected);
          }
        });
      }
    }

    throwsTest(`${hasherName} invalid path type`, hasher, {
      path: undefined,
      seed: 0,
    });
    throwsTest(`${hasherName} invalid seed type`, hasher, {
      path: '123',
      seed: '123',
    });

    for (const type of types) {
      test(`${hasherName} throws when file doesn't exist`, () => {
        expect(() =>
          hasher({ path: './test_data/should-not-exist', seed: 1, type }),
        ).toThrow();
      });
    }
  }

  variantTests('xxhash32', xxhash32.file, [[1, 1945663033]]);
  variantTests('xxhash64', xxhash64.file, [
    [1, BigInt('17740802669433987345')],
  ]);
  variantTests('xxhash3', xxhash3.file, [
    [0, BigInt('12531405323377630900')],
    [1, BigInt('8310716519890529791')],
  ]);
  variantTests('xxhash3_128', xxhash3_128.file, [
    [0, BigInt('193898327962634967863812790837365759668')],
    [1, BigInt('132161492315031615344357334049880780287')],
  ]);
});

describe('state hashing', () => {
  const testData = Uint8Array.from([97, 98, 99, 100]);

  function variantTests<S extends number | bigint, T extends number | bigint>(
    hasherName: string,
    createState: (seed?: S) => XxHashState<T>,
    input: [S | undefined, Uint8Array[], T][],
  ) {
    for (const [seed, arrays, expected] of input) {
      test(`${hasherName} state hashing`, () => {
        const state = createState(seed);

        for (const array of arrays) {
          state.update(array);
        }

        const actual = state.result();

        expect(actual).toEqual(expected);
      });
    }
  }

  variantTests('xxhash32', xxhash32.createState, [[0, [testData], 0xa3643705]]);
  variantTests('xxhash64', xxhash64.createState, [
    [1, [testData], BigInt('0xf5dcbd6dee3c9553')],
  ]);
  variantTests('xxhash3', xxhash3.createState, [
    [undefined, [testData], BigInt('7248448420886124688')],
  ]);
  variantTests('xxhash3_128', xxhash3_128.createState, [
    [0, [testData], BigInt('187978674736816916650311503294403523901')],
  ]);
});
