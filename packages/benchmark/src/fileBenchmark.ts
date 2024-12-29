import { Bench } from 'tinybench';
import {
  xxhash3 as minXxHash3,
  FileHashingOptions,
} from 'xxhash-bindings-min/index.ts';
import { xxhash3 as allXxHash3 } from 'xxhash-bindings-allnative/index.ts';
import { generateRandomFileContent } from './randomDataGenerator.js';
import { KB, MB, TEST_DATA_PATH } from './constants.ts';
import fs from 'fs';

const fileInfos: { name: string; size: number }[] = [
  { name: 'gb1', size: 1024 * MB },
  { name: 'mb5', size: 5 * MB },
  { name: 'kb1', size: KB },
];

export const name = 'file';

export async function run(): Promise<Bench> {
  console.log('Checking data file');

  await Promise.all(
    fileInfos.map(async ({ name, size }) => {
      const path = `${TEST_DATA_PATH}/${name}`;

      if (!fs.existsSync(path)) {
        await generateRandomFileContent(path, size);
      }
    }),
  );

  console.log('Starting benchmark');

  const bench = new Bench({
    warmupIterations: 3,
    iterations: 15,
  });

  for (const { name } of fileInfos) {
    const path = `${TEST_DATA_PATH}/${name}`;

    const mapOptions: FileHashingOptions<number> = {
      path,
      seed: 1,
      preferMap: true,
    };

    const blockOptions: FileHashingOptions<number> = {
      path,
      seed: 1,
      preferMap: false,
    };

    bench
      .add(`all map (${name})`, () => {
        allXxHash3.file(mapOptions);
      })
      .add(`all block (${name})`, () => {
        allXxHash3.file(blockOptions);
      })
      .add(`min block (${name})`, () => {
        minXxHash3.file(blockOptions);
      });
  }

  return bench;
}

export default { name, run };
