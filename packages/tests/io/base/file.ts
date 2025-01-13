import { test, expect, describe } from 'vitest';
import fs from 'fs';
import { Lib, VariantName, testData, variantNames } from '@/utils';
import minimum, { FileHashOptions, XxVariantName } from 'xxhash-bindings-min';
import allnative from 'xxhash-bindings-allnative';
import { numberWithBigint } from './helpers';

const preferMapValues = [undefined, false, true];

type GenericFileHasher = (
  options: FileHashOptions<number>,
) => Promise<number | bigint>;

type SetupTestsContext = {
  getFileFactory: (lib: Lib, variant: XxVariantName) => GenericFileHasher;

  expectToThrowError(
    options: FileHashOptions<number>,
    hasher: GenericFileHasher,
    error?: Error,
  ): Promise<void>;
};

export function setupTests(context: SetupTestsContext) {
  setupTestsOnLib(context, minimum, 'minimum');
  setupTestsOnLib(context, allnative, 'allnative');
}

function setupTestsOnLib(
  { getFileFactory, expectToThrowError }: SetupTestsContext,
  lib: Lib,
  libName: string,
) {
  describe(`${libName} file`, () => {
    test.each<[VariantName, number | bigint]>([
      ['xxhash32', 52811677],
      ['xxhash64', BigInt('18153045472420481988')],
      ['xxhash3', BigInt('12531405323377630900')],
      ['xxhash3_128', BigInt('193898327962634967863812790837365759668')],
    ])('no seed', async (name, expected) => {
      const file = getFileFactory(lib, name);

      for (const preferMap of preferMapValues) {
        const baseOptions = { path: testData('image1.png'), preferMap };

        for (const options of [
          baseOptions,
          { ...baseOptions, seed: 0 },
          { ...baseOptions, seed: undefined },
        ]) {
          const actual = await file(options);

          expect(actual).toBe(expected);
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
      [
        'xxhash3_128',
        0,
        100,
        BigInt('199877537612181396746579086691350335749'),
      ],
      [
        'xxhash3_128',
        100,
        200,
        BigInt('332463148870137050248467284077638561955'),
      ],
      ['xxhash3_128', 0, 0, BigInt('204254712233039002205064565430793619839')],
      [
        'xxhash3_128',
        0,
        10_000_000_000,
        BigInt('193898327962634967863812790837365759668'),
      ],
    ])('file part no seed', async (name, offset, length, expected) => {
      const file = getFileFactory(lib, name);

      for (const offsetOrBigint of numberWithBigint(offset)) {
        for (const lengthOrBigint of numberWithBigint(length)) {
          for (const preferMap of preferMapValues) {
            const baseOptions = {
              path: testData('image1.png'),
              preferMap,
              offset: offsetOrBigint,
              length: lengthOrBigint,
            };

            for (const options of [
              baseOptions,
              { ...baseOptions, seed: 0 },
              { ...baseOptions, seed: undefined },
            ]) {
              const actual = await file(options);

              expect(actual).toBe(expected);
            }
          }
        }
      }
    });

    test.each<[VariantName, number | bigint]>([
      ['xxhash32', 1945663033],
      ['xxhash64', BigInt('17740802669433987345')],
      ['xxhash3', BigInt('8310716519890529791')],
      ['xxhash3_128', BigInt('132161492315031615344357334049880780287')],
    ])('with seed', async (name, expected) => {
      const file = getFileFactory(lib, name);

      for (const preferMap of preferMapValues) {
        const actual = await file({
          path: testData('image1.png'),
          seed: 1,
          preferMap,
        });

        expect(actual).toBe(expected);
      }
    });

    test.each<[XxVariantName, number | bigint]>([
      ['xxhash32', 46947589],
      ['xxhash64', BigInt('17241709254077376921')],
      ['xxhash3', BigInt('3244421341483603138')],
      ['xxhash3_128', BigInt('204254712233039002205064565430793619839')],
    ])('empty file no seed', async (name, expected) => {
      const file = getFileFactory(lib, name);

      for (const preferMap of preferMapValues) {
        const actual = await file({ path: testData('emptyfile'), preferMap });

        expect(actual).toBe(expected);
      }
    });

    test.each<[XxVariantName, number | bigint]>([
      ['xxhash32', 3068971186],
      ['xxhash64', BigInt('13237225503670494420')],
      ['xxhash3', BigInt('7335560060985733464')],
      ['xxhash3_128', BigInt('296734076633237196744344171427223105880')],
    ])('one byte file no seed', async (name, expected) => {
      const file = getFileFactory(lib, name);

      for (const preferMap of preferMapValues) {
        const actual = await file({ path: testData('onebyte'), preferMap });

        expect(actual).toBe(expected);
      }
    });

    test
      .runIf(fs.existsSync('/dev/zero'))
      .each<[XxVariantName, number | bigint]>([
        ['xxhash32', 593485017],
        ['xxhash64', BigInt('8040990501003754519')],
        ['xxhash3', BigInt('665452966430363425')],
        ['xxhash3_128', BigInt('281489807592584962896215940306331225889')],
      ])('/dev/zero test', async (name, expected) => {
      const file = getFileFactory(lib, name);

      for (const preferMap of preferMapValues) {
        const actual = await file({
          path: '/dev/zero',
          preferMap,
          length: 128,
        });

        expect(actual).toBe(expected);
      }
    });

    test.each(variantNames.map((name) => [name]))(
      'throws on invalid path',
      async (name) => {
        const file = getFileFactory(lib, name);

        await expectToThrowError({ path: 123 as unknown as string }, file);
      },
    );

    test.each<[VariantName, string]>([
      ['xxhash32', 'number or undefined'],
      ['xxhash64', 'number, bigint or undefined'],
      ['xxhash3', 'number, bigint or undefined'],
      ['xxhash3_128', 'number, bigint or undefined'],
    ])('throws on invalid seed', async (name, expected) => {
      const file = getFileFactory(lib, name);

      await expectToThrowError(
        { path: testData('image1.png'), seed: '1' as unknown as number },
        file,
        Error(`Expected type of the property "seed" is ${expected}`),
      );
    });

    test.each(variantNames.map((name) => [name]))(
      'throws on invalid preferMap',
      async (name) => {
        const file = getFileFactory(lib, name);

        await expectToThrowError(
          {
            path: testData('image1.png'),
            seed: 1,
            preferMap: 100 as unknown as boolean,
          },
          file,
          Error(
            'Expected type of the property "preferMap" is boolean or undefined',
          ),
        );
      },
    );

    test.each(variantNames.map((name) => [name]))(
      'throws on non-existent path',
      async (name) => {
        const file = getFileFactory(lib, name);

        await expectToThrowError({ path: './.should_not_exist' }, file);
      },
    );

    test.each([
      [0.5, 1],
      [0, 1.5],
      [-1, -1],
      [-1, 2],
      [0, -1],
      [Number.NaN, 1],
      [1, Number.NaN],
      [Number.POSITIVE_INFINITY, 2],
      [Number.NEGATIVE_INFINITY, 3],
      [2, Number.POSITIVE_INFINITY],
      [3, Number.NEGATIVE_INFINITY],
      [0n, -2n],
      [-1n, -1n],
      [-1n, 2n],
      [0n, -1n],
    ])('throws on invalid bounds', async (offset, length) => {
      for (const name of variantNames) {
        const file = getFileFactory(lib, name);

        for (const preferMap of preferMapValues) {
          await expectToThrowError(
            {
              path: testData('image1.png'),
              offset,
              length,
              preferMap,
            },
            file,
          );
        }
      }
    });
  });
}
