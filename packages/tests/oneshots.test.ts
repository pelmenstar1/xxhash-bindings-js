import { expect, test } from 'vitest';
import { HasherName, hideArgumentTypes, libs } from './utils';

const testData = Uint8Array.from([97, 98, 99, 100]);

test.each<[HasherName, number | bigint]>([
  ['xxhash32', 46947589],
  ['xxhash64', BigInt('17241709254077376921')],
  ['xxhash3', BigInt('3244421341483603138')],
  ['xxhash3_128', BigInt('204254712233039002205064565430793619839')],
])('empty buffer', (name, expected) => {
  for (const lib of libs) {
    const { oneshot } = lib[name];

    const buffer = Uint8Array.of();

    expect(oneshot(buffer)).toBe(expected);
    expect(oneshot(buffer, 0)).toBe(expected);
    expect(oneshot(buffer, undefined)).toBe(expected);
  }
});

test.each<[HasherName, number | bigint]>([
  ['xxhash32', 187479954],
  ['xxhash64', BigInt('15397730242686860875')],
  ['xxhash3', BigInt('5604079703740606211')],
  ['xxhash3_128', BigInt('288641663974058300363650943496620614605')],
])('empty buffer with seed', (name, expected) => {
  for (const lib of libs) {
    const { oneshot } = lib[name];

    const buffer = Uint8Array.of();

    expect(oneshot(buffer, 1)).toBe(expected);
  }
});

test.each<[HasherName, number | bigint]>([
  ['xxhash32', 0xa3643705],
  ['xxhash64', BigInt('15997673941747208908')],
  ['xxhash3', BigInt('7248448420886124688')],
  ['xxhash3_128', BigInt('187978674736816916650311503294403523901')],
])('no seed', (name, expected) => {
  for (const lib of libs) {
    const { oneshot } = lib[name];

    expect(oneshot(testData)).toBe(expected);
    expect(oneshot(testData, 0)).toBe(expected);
    expect(oneshot(testData, undefined)).toBe(expected);
  }
});

test.each<[HasherName, number | bigint]>([
  ['xxhash32', 3194890655],
  ['xxhash64', BigInt('0xf5dcbd6dee3c9553')],
  ['xxhash3', BigInt('13053297174767443700')],
  ['xxhash3_128', BigInt('233827487414958832244112154636312345382')],
])('with seed', (name, expected) => {
  for (const lib of libs) {
    const { oneshot } = lib[name];

    const actual = oneshot(testData, 1);

    expect(actual).toEqual(expected);
  }
});

test.each<[HasherName]>([
  ['xxhash32'],
  ['xxhash64'],
  ['xxhash3'],
  ['xxhash3_128'],
])('throws on invalid arguments', (name) => {
  for (const lib of libs) {
    const { oneshot } = lib[name];

    const unsafeHasher = hideArgumentTypes<
      [Uint8Array, number | undefined],
      number | bigint
    >(oneshot);

    expect(() => unsafeHasher(Uint8Array.of(), '123')).toThrowError();
    expect(() => unsafeHasher(0 as unknown as Uint8Array, 1)).toThrowError();
  }
});
