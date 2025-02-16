import { expect, test } from 'vitest';
import lib, { XxVariantName } from 'xxhash-bindings';

const testData = Uint8Array.from([97, 98, 99, 100]);

test.each<[XxVariantName, number | bigint]>([
  ['xxhash32', 46947589],
  ['xxhash64', BigInt('17241709254077376921')],
  ['xxhash3', BigInt('3244421341483603138')],
  ['xxhash3_128', BigInt('204254712233039002205064565430793619839')],
])('empty buffer', (name, expected) => {
  const { oneshot } = lib[name];

  const buffer = Uint8Array.of();

  expect(oneshot(buffer)).toBe(expected);
  expect(oneshot(buffer, 0)).toBe(expected);
  expect(oneshot(buffer, undefined)).toBe(expected);
});

test.each<[XxVariantName, number | bigint]>([
  ['xxhash32', 187479954],
  ['xxhash64', BigInt('15397730242686860875')],
  ['xxhash3', BigInt('5604079703740606211')],
  ['xxhash3_128', BigInt('288641663974058300363650943496620614605')],
])('empty buffer with seed', (name, expected) => {
  const { oneshot } = lib[name];

  const buffer = Uint8Array.of();

  expect(oneshot(buffer, 1)).toBe(expected);
});

test.each<[XxVariantName, number | bigint]>([
  ['xxhash32', 0xa3643705],
  ['xxhash64', BigInt('15997673941747208908')],
  ['xxhash3', BigInt('7248448420886124688')],
  ['xxhash3_128', BigInt('187978674736816916650311503294403523901')],
])('no seed', (name, expected) => {
  const { oneshot } = lib[name];

  expect(oneshot(testData)).toBe(expected);
  expect(oneshot(testData, 0)).toBe(expected);
  expect(oneshot(testData, undefined)).toBe(expected);
});

test.each<[XxVariantName, number | bigint]>([
  ['xxhash32', 3194890655],
  ['xxhash64', BigInt('0xf5dcbd6dee3c9553')],
  ['xxhash3', BigInt('13053297174767443700')],
  ['xxhash3_128', BigInt('233827487414958832244112154636312345382')],
])('with seed', (name, expected) => {
  const { oneshot } = lib[name];

  const actual = oneshot(testData, 1);

  expect(actual).toEqual(expected);
});

test.each<[XxVariantName, string]>([
  ['xxhash32', 'number or undefined'],
  ['xxhash64', 'number, bigint or undefined'],
  ['xxhash3', 'number, bigint or undefined'],
  ['xxhash3_128', 'number, bigint or undefined'],
])('throws on invalid seed', (name, expected) => {
  const { oneshot } = lib[name];

  expect(() =>
    oneshot(Uint8Array.of(), '123' as unknown as number),
  ).toThrowError(Error(`Expected type of the parameter "seed" is ${expected}`));
});

test.each<[XxVariantName]>([
  ['xxhash32'],
  ['xxhash64'],
  ['xxhash3'],
  ['xxhash3_128'],
])('throws on invalid seed', (name) => {
  const { oneshot } = lib[name];

  expect(() => oneshot(1 as unknown as Uint8Array, 123)).toThrowError(
    Error('Expected type of the parameter "data" is Uint8Array'),
  );
});
