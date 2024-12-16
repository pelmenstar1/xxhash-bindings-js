import { test, expect } from 'vitest';
import { xxhash32, xxhash64, xxhash3, xxhash3_128 } from '..';

const testData1 = Uint8Array.from([97, 98, 99, 100]);
const testData2 = Uint8Array.from([101, 102, 103, 104]);

test.each([
  [xxhash32.createState, 46947589],
  [xxhash64.createState, BigInt('17241709254077376921')],
  [xxhash3.createState, BigInt('3244421341483603138')],
  [xxhash3_128.createState, BigInt('204254712233039002205064565430793619839')],
])('no blocks no seed', (hasher, expected) => {
  for (const create of [
    () => hasher(),
    () => hasher(undefined),
    () => hasher(0),
  ]) {
    const state = create();

    const actual = state.result();
    expect(actual).toBe(expected);
  }
});

test.each([
  [xxhash32.createState, 46947589],
  [xxhash64.createState, BigInt('17241709254077376921')],
  [xxhash3.createState, BigInt('3244421341483603138')],
  [xxhash3_128.createState, BigInt('204254712233039002205064565430793619839')],
])('empty block no seed', (hasher, expected) => {
  for (const create of [
    () => hasher(),
    () => hasher(undefined),
    () => hasher(0),
  ]) {
    const state = create();
    state.update(Uint8Array.of());

    const actual = state.result();
    expect(actual).toBe(expected);
  }
});

test.each([
  [xxhash32.createState, 187479954],
  [xxhash64.createState, BigInt('15397730242686860875')],
  [xxhash3.createState, BigInt('5604079703740606211')],
  [xxhash3_128.createState, BigInt('288641663974058300363650943496620614605')],
])('no blocks with seed', (hasher, expected) => {
  const state = hasher(1);

  const actual = state.result();
  expect(actual).toBe(expected);
});

test.each([
  [xxhash32.createState, 0xa3643705],
  [xxhash64.createState, BigInt('15997673941747208908')],
  [xxhash3.createState, BigInt('7248448420886124688')],
  [xxhash3_128.createState, BigInt('187978674736816916650311503294403523901')],
])('one block no seed', (hasher, expected) => {
  for (const create of [
    () => hasher(),
    () => hasher(undefined),
    () => hasher(0),
  ]) {
    const state = create();
    state.update(testData1);

    const actual = state.result();
    expect(actual).toBe(expected);
  }
});

test.each([
  [xxhash32.createState, 0xa3643705],
  [xxhash64.createState, BigInt('15997673941747208908')],
  [xxhash3.createState, BigInt('7248448420886124688')],
  [xxhash3_128.createState, BigInt('187978674736816916650311503294403523901')],
])('one block with seed', (hasher, expected) => {
  const state = hasher();
  state.update(testData1);

  const actual = state.result();
  expect(actual).toBe(expected);
});

test.each([
  [xxhash32.createState, 196331195],
  [xxhash64.createState, BigInt('4238821247360054455')],
  [xxhash3.createState, BigInt('8017998777839871107')],
  [xxhash3_128.createState, BigInt('290780027203923156590853666013685747474')],
])('two blocks no seed', (hasher, expected) => {
  for (const blocks of [
    [testData1, testData2],
    [Uint8Array.of(...testData1, ...testData2)],
  ]) {
    for (const create of [
      () => hasher(),
      () => hasher(undefined),
      () => hasher(0),
    ]) {
      const state = create();

      for (const block of blocks) {
        state.update(block);
      }

      const actual = state.result();
      expect(actual).toBe(expected);
    }
  }
});

test.each([
  [xxhash32.createState, 3058100195],
  [xxhash64.createState, BigInt('9440972358994176609')],
  [xxhash3.createState, BigInt('8998154142266437692')],
  [xxhash3_128.createState, BigInt('1164931414052291738040848678873537683')],
])('two blocks with seed', (hasher, expected) => {
  for (const blocks of [
    [testData1, testData2],
    [Uint8Array.of(...testData1, ...testData2)],
  ]) {
    const state = hasher(1);

    for (const block of blocks) {
      state.update(block);
    }

    const actual = state.result();
    expect(actual).toBe(expected);
  }
});

const hashers = [
  [xxhash32.createState],
  [xxhash64.createState],
  [xxhash3.createState],
  [xxhash3_128.createState],
] as const;

test.each(hashers)('throws on invalid seed', (hasher) => {
  expect(() => hasher('123' as unknown as number)).toThrowError();
});

test.each(hashers)('throws on invalid array', (hasher) => {
  const state = hasher();

  expect(() => state.update(0 as unknown as Uint8Array)).toThrowError();
});
