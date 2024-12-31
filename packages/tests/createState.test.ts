import { test, expect } from 'vitest';
import { Lib, VariantName, libs, variantNames } from './utils';
import { XxHashState } from 'xxhash-bindings-min';

const testData1 = Uint8Array.from([97, 98, 99, 100]);
const testData2 = Uint8Array.from([101, 102, 103, 104]);

function forEachNoSeedState(
  createState: Lib[VariantName]['createState'],
  block: (state: XxHashState<number | bigint>) => void,
) {
  for (const state of [createState(), createState(undefined), createState(0)]) {
    block(state);
  }
}

test.each<[VariantName, number | bigint]>([
  ['xxhash32', 46947589],
  ['xxhash64', BigInt('17241709254077376921')],
  ['xxhash3', BigInt('3244421341483603138')],
  ['xxhash3_128', BigInt('204254712233039002205064565430793619839')],
])('no blocks no seed', (name, expected) => {
  for (const lib of libs) {
    const { createState } = lib[name];

    forEachNoSeedState(createState, (state) => {
      const actual = state.result();
      expect(actual).toBe(expected);
    });
  }
});

test.each<[VariantName, number | bigint]>([
  ['xxhash32', 46947589],
  ['xxhash64', BigInt('17241709254077376921')],
  ['xxhash3', BigInt('3244421341483603138')],
  ['xxhash3_128', BigInt('204254712233039002205064565430793619839')],
])('empty block no seed', (name, expected) => {
  for (const lib of libs) {
    const { createState } = lib[name];

    forEachNoSeedState(createState, (state) => {
      state.update(Uint8Array.of());

      const actual = state.result();
      expect(actual).toBe(expected);
    });
  }
});

test.each<[VariantName, number | bigint]>([
  ['xxhash32', 187479954],
  ['xxhash64', BigInt('15397730242686860875')],
  ['xxhash3', BigInt('5604079703740606211')],
  ['xxhash3_128', BigInt('288641663974058300363650943496620614605')],
])('no blocks with seed', (name, expected) => {
  for (const lib of libs) {
    const { createState } = lib[name];

    const state = createState(1);

    const actual = state.result();
    expect(actual).toBe(expected);
  }
});

test.each<[VariantName, number | bigint]>([
  ['xxhash32', 0xa3643705],
  ['xxhash64', BigInt('15997673941747208908')],
  ['xxhash3', BigInt('7248448420886124688')],
  ['xxhash3_128', BigInt('187978674736816916650311503294403523901')],
])('one block no seed', (name, expected) => {
  for (const lib of libs) {
    const { createState } = lib[name];

    forEachNoSeedState(createState, (state) => {
      state.update(testData1);

      const actual = state.result();
      expect(actual).toBe(expected);
    });
  }
});

test.each<[VariantName, number | bigint]>([
  ['xxhash32', 3194890655],
  ['xxhash64', BigInt('17716243314014721363')],
  ['xxhash3', BigInt('13053297174767443700')],
  ['xxhash3_128', BigInt('233827487414958832244112154636312345382')],
])('one block with seed', (name, expected) => {
  for (const lib of libs) {
    const { createState } = lib[name];

    const state = createState(1);
    state.update(testData1);

    const actual = state.result();
    expect(actual).toBe(expected);
  }
});

test.each<[VariantName, number | bigint, number | bigint]>([
  ['xxhash32', 0xa3643705, 46947589],
  ['xxhash64', BigInt('15997673941747208908'), BigInt('17241709254077376921')],
  ['xxhash3', BigInt('7248448420886124688'), BigInt('3244421341483603138')],
  [
    'xxhash3_128',
    BigInt('187978674736816916650311503294403523901'),
    BigInt('204254712233039002205064565430793619839'),
  ],
])('one block no seed reset', (name, before, after) => {
  for (const lib of libs) {
    const { createState } = lib[name];

    forEachNoSeedState(createState, (state) => {
      state.update(testData1);

      const actual = state.result();
      expect(actual).toBe(before);

      state.reset();

      const actual2 = state.result();
      expect(actual2).toBe(after);
    });
  }
});

test.each<[VariantName, number | bigint, number | bigint]>([
  ['xxhash32', 3194890655, 187479954],
  ['xxhash64', BigInt('17716243314014721363'), BigInt('15397730242686860875')],
  ['xxhash3', BigInt('13053297174767443700'), BigInt('5604079703740606211')],
  [
    'xxhash3_128',
    BigInt('233827487414958832244112154636312345382'),
    BigInt('288641663974058300363650943496620614605'),
  ],
])('one block with seed reset', (name, before, after) => {
  for (const lib of libs) {
    const { createState } = lib[name];

    const state = createState(1);
    state.update(testData1);

    const actual = state.result();
    expect(actual).toBe(before);

    state.reset();

    const actual2 = state.result();
    expect(actual2).toBe(after);
  }
});

test.each<[VariantName, number | bigint]>([
  ['xxhash32', 3194890655],
  ['xxhash64', BigInt('17716243314014721363')],
  ['xxhash3', BigInt('13053297174767443700')],
  ['xxhash3_128', BigInt('233827487414958832244112154636312345382')],
])('one block with seed', (name, expected) => {
  for (const lib of libs) {
    const { createState } = lib[name];

    const state = createState(1);
    state.update(testData1);

    const actual = state.result();
    expect(actual).toBe(expected);
  }
});

test.each([
  ['xxhash32', 196331195],
  ['xxhash64', BigInt('4238821247360054455')],
  ['xxhash3', BigInt('8017998777839871107')],
  ['xxhash3_128', BigInt('290780027203923156590853666013685747474')],
])('two blocks no seed', (name, expected) => {
  for (const lib of libs) {
    const { createState } = lib[name];

    for (const blocks of [
      [testData1, testData2],
      [Uint8Array.of(...testData1, ...testData2)],
    ]) {
      forEachNoSeedState(createState, (state) => {
        for (const block of blocks) {
          state.update(block);
        }

        const actual = state.result();
        expect(actual).toBe(expected);
      });
    }
  }
});

test.each([
  ['xxhash32', 3058100195],
  ['xxhash64', BigInt('9440972358994176609')],
  ['xxhash3', BigInt('8998154142266437692')],
  ['xxhash3_128', BigInt('1164931414052291738040848678873537683')],
])('two blocks with seed', (name, expected) => {
  for (const lib of libs) {
    const { createState } = lib[name];

    for (const blocks of [
      [testData1, testData2],
      [Uint8Array.of(...testData1, ...testData2)],
    ]) {
      const state = createState(1);

      for (const block of blocks) {
        state.update(block);
      }

      const actual = state.result();
      expect(actual).toBe(expected);
    }
  }
});

test.each<[VariantName, string]>([
  ['xxhash32', 'number or undefined'],
  ['xxhash64', 'number, bigint or undefined'],
  ['xxhash3', 'number, bigint or undefined'],
  ['xxhash3_128', 'number, bigint or undefined'],
])('throws on invalid seed', (name, expected) => {
  for (const lib of libs) {
    const { createState } = lib[name];

    expect(() => createState('123' as unknown as number)).toThrowError(
      Error(`Expected type of the parameter "seed" is ${expected}`),
    );
  }
});

test.each(variantNames.map((name) => [name]))(
  'throws on invalid array',
  (name) => {
    for (const lib of libs) {
      const { createState } = lib[name];
      const state = createState();

      expect(() => state.update(0 as unknown as Uint8Array)).toThrowError(
        Error('Expected type of the parameter "data" is Uint8Array'),
      );
    }
  },
);
