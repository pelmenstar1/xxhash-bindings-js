import { expect, test } from 'vitest';
import { xxhash32, xxhash64, xxhash3, xxhash3_128 } from '..';
import { hideArgumentTypes } from './utils';

const testData = Uint8Array.from([97, 98, 99, 100]);

test.each([
  [xxhash32.oneshot, 46947589],
  [xxhash64.oneshot, BigInt('17241709254077376921')],
  [xxhash3.oneshot, BigInt('3244421341483603138')],
  [xxhash3_128.oneshot, BigInt('204254712233039002205064565430793619839')],
])('empty buffer', (hasher, expected) => {
  const buffer = Uint8Array.of();

  expect(hasher(buffer)).toBe(expected);
  expect(hasher(buffer, 0)).toBe(expected);
  expect(hasher(buffer, undefined)).toBe(expected);
});

test.each([
  [xxhash32.oneshot, 187479954],
  [xxhash64.oneshot, BigInt('15397730242686860875')],
  [xxhash3.oneshot, BigInt('5604079703740606211')],
  [xxhash3_128.oneshot, BigInt('288641663974058300363650943496620614605')],
])('empty buffer with seed', (hasher, expected) => {
  const buffer = Uint8Array.of();

  expect(hasher(buffer, 1)).toBe(expected);
});

test.each([
  [xxhash32.oneshot, 0xa3643705],
  [xxhash64.oneshot, BigInt('15997673941747208908')],
  [xxhash3.oneshot, BigInt('7248448420886124688')],
  [xxhash3_128.oneshot, BigInt('187978674736816916650311503294403523901')],
])('no seed', (hasher, expected) => {
  expect(hasher(testData)).toBe(expected);
  expect(hasher(testData, 0)).toBe(expected);
  expect(hasher(testData, undefined)).toBe(expected);
});

test.each([
  [xxhash32.oneshot, 3194890655],
  [xxhash64.oneshot, BigInt('0xf5dcbd6dee3c9553')],
  [xxhash3.oneshot, BigInt('13053297174767443700')],
  [xxhash3_128.oneshot, BigInt('233827487414958832244112154636312345382')],
])('with seed', (hasher, expected) => {
  const actual = hasher(testData, 1);

  expect(actual).toEqual(expected);
});

test.each([
  [xxhash32.oneshot],
  [xxhash64.oneshot],
  [xxhash3.oneshot],
  [xxhash3_128.oneshot],
])('throws on invalid arguments', (hasher) => {
  const unsafeHasher = hideArgumentTypes<
    [Uint8Array, number | undefined],
    number | bigint
  >(hasher);

  expect(() => unsafeHasher(Uint8Array.of(), '123')).toThrowError();
  expect(() => unsafeHasher(0 as unknown as Uint8Array, 1)).toThrowError();
});
