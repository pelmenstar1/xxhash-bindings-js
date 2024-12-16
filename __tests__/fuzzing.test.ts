import { test, expect } from 'vitest';
import { xxhash32, xxhash64, xxhash3, xxhash3_128, FileHashingType } from '..';
import fs from 'fs';

const TEST_FILE_PATH = './test_data/image1.png';
const hashTypes = [FileHashingType.MAP, FileHashingType.BLOCK];

test('file oneshot fuzzing', async () => {
  const fileContent = await fs.promises.readFile(TEST_FILE_PATH);

  for (const type of hashTypes) {
    for (const seed of [1, 2]) {
      for (const hashVariant of [xxhash32, xxhash64, xxhash3, xxhash3_128]) {
        for (let offset = 0; offset < fileContent.length; offset += 4096) {
          for (const length of [1, 128, 1024, 1025]) {
            const fileResult = hashVariant.file({
              path: TEST_FILE_PATH,
              offset,
              length,
              type,
              seed,
            });

            const part = fileContent.subarray(offset, offset + length);

            const oneshotResult = hashVariant.oneshot(part, seed);

            expect(fileResult).toBe(oneshotResult);
          }
        }
      }
    }
  }
});

test('file oneshot state fuzzing', async () => {
  const fileContent = await fs.promises.readFile(TEST_FILE_PATH);

  for (const seed of [1, 2]) {
    for (const hashVariant of [xxhash32, xxhash64, xxhash3, xxhash3_128]) {
      const oneshotHash = hashVariant.oneshot(fileContent, seed);

      const state = hashVariant.createState(seed);

      for (let offset = 0; offset < fileContent.length; offset += 4096) {
        state.update(fileContent.subarray(offset, offset + 4096));
      }

      const stateHash = state.result();

      for (const type of hashTypes) {
        const fileHash = hashVariant.file({ path: TEST_FILE_PATH, seed, type });

        expect(oneshotHash).toBe(fileHash);
        expect(fileHash).toBe(stateHash);
      }
    }
  }
});
