import { describe, expect, test } from "vitest";
import { xxhash32, xxhash64, xxhash3, xxhash3_128, FileHashingType, FileHashingOptions } from "./index.js";

type AnyArray<Arr extends any[]> = {
  [Index in keyof Arr]: any;
}

type Args<F> = F extends (...args: infer Args) => unknown ? Args : never;
type AnyArgs<F> = AnyArray<Args<F>>;

type AnyFunction = (...args: any[]) => any;

function hideArgumentTypes<F extends AnyFunction>(func: F, ...args: AnyArgs<F>): ReturnType<F> {
  return func(...args);
}

describe("oneshots", () => {
  const testData = Uint8Array.from([97, 98, 99, 100])

  test("xxhash32", () => {
    expect(xxhash32.oneshot(testData, 0)).toBe(0xa3643705);
  });

  test("xxhash64", () => {
    expect(xxhash64.oneshot(testData, 1)).toBe(BigInt("0xf5dcbd6dee3c9553"));
  });

  test("xxhash3", () => {
    const expected = BigInt("7248448420886124688");

    expect(xxhash3.oneshot(testData)).toBe(expected);
    expect(xxhash3.oneshot(testData, 0)).toBe(expected);
    expect(xxhash3.oneshot(testData, undefined)).toBe(expected);
  });

  test("xxhash3 with seed", () => {
    expect(xxhash3.oneshot(testData, 1)).toBe(BigInt("13053297174767443700"));
  });

  test("xxhash3 (128)", () => {
    const expected = BigInt("187978674736816916650311503294403523901");

    expect(xxhash3_128.oneshot(testData)).toBe(expected);
    expect(xxhash3_128.oneshot(testData, 0)).toBe(expected);
    expect(xxhash3_128.oneshot(testData, undefined)).toBe(expected);
  });

  test("xxhash3 with seed (128)", () => {
    expect(xxhash3_128.oneshot(testData, 1)).toBe(BigInt("233827487414958832244112154636312345382"));
  });

  function throwsTest<F extends AnyFunction>(testName: string, hasher: F, ...args: AnyArgs<F>) {
    test(testName, () => {
      expect(() => hideArgumentTypes(hasher, ...args)).toThrow();
    });
  }

  function throwsInvalidTypeTest(functionName: string, hasher: (buffer: Uint8Array, seed: number) => unknown) {
    throwsTest(`${functionName} invalid buffer type`, hasher, undefined, 0);
    throwsTest(`${functionName} invalid seed type`, hasher, Uint8Array.of(0), "123");
  }

  throwsInvalidTypeTest("xxhash32", xxhash32.oneshot);
  throwsInvalidTypeTest("xxhash64", xxhash64.oneshot);
  throwsInvalidTypeTest("xxhash3", xxhash3.oneshot);
  throwsInvalidTypeTest("xxhash3_128", xxhash3_128.oneshot);
});

describe("file hashing", () => {
  const TEST_FILE_PATH = "./test_data/image1.png";

  test("xxhash32", () => {
    expect(xxhash32.file({ path: TEST_FILE_PATH, seed: 1, type: FileHashingType.MAP })).toBe(1945663033)
  })

  test("xxhash64", () => {
    expect(xxhash64.file({ path: TEST_FILE_PATH, seed: 1 })).toBe(BigInt("17740802669433987345"))
  })

  test("xxhash3", () => {
    const expected = BigInt("12531405323377630900");

    expect(xxhash3.file({ path: TEST_FILE_PATH })).toBe(expected)
    expect(xxhash3.file({ path: TEST_FILE_PATH, seed: 0 })).toBe(expected)
    expect(xxhash3.file({ path: TEST_FILE_PATH, seed: undefined })).toBe(expected)
  })

  test("xxhash3 with seed", () => {
    expect(xxhash3.file({ path: TEST_FILE_PATH, seed: 1 })).toBe(BigInt("8310716519890529791"))
  })

  test("xxhash3_128", () => {
    const expected = BigInt("193898327962634967863812790837365759668");

    expect(xxhash3_128.file({ path: TEST_FILE_PATH })).toBe(expected)
    expect(xxhash3_128.file({ path: TEST_FILE_PATH, seed: 0 })).toBe(expected)
    expect(xxhash3_128.file({ path: TEST_FILE_PATH, seed: undefined })).toBe(expected)
  })

  test("xxhash3_128 with seed", () => {
    expect(xxhash3_128.file({ path: TEST_FILE_PATH, seed: 1 })).toBe(BigInt("132161492315031615344357334049880780287"))
  });

  function throwsTest<F extends AnyFunction>(testName: string, hasher: F, ...args: AnyArgs<F>) {
    test(testName, () => {
      expect(() => hideArgumentTypes(hasher, ...args)).toThrow();
    });
  }

  function throwsInvalidTypeTest(functionName: string, hasher: (options: FileHashingOptions<number>) => unknown) {
    throwsTest(`${functionName} invalid path type`, hasher, { path: undefined, seed: 0 });
    throwsTest(`${functionName} invalid seed type`, hasher, { path: "123", seed: "123" });
  }

  throwsInvalidTypeTest("xxhash32", xxhash32.file);
  throwsInvalidTypeTest("xxhash64", xxhash64.file);
  throwsInvalidTypeTest("xxhash3", xxhash3.file);
  throwsInvalidTypeTest("xxhash3_128", xxhash3_128.file);

  function throwsWhenNoFileTest(functionName: string, hasher: (options: FileHashingOptions<number>) => unknown) {
    test(`${functionName} throws when file doesn't exist`, () => {
      expect(() => hasher({path: "./test_data/should-not-exist", seed: 1})).toThrow()
    })
  }

  throwsWhenNoFileTest("xxhash32", xxhash32.file);
  throwsWhenNoFileTest("xxhash64", xxhash64.file);
  throwsWhenNoFileTest("xxhash3", xxhash3.file);
  throwsWhenNoFileTest("xxhash3_128", xxhash3_128.file);
})