import { Bench } from 'tinybench';
import { generateRandomFileContent } from './randomDataGenerator.js';
import { KB, MB, TEST_DATA_PATH } from './constants.ts';
import fs from 'fs';
import { FileHashOptions, xxhash3 } from 'xxhash-bindings';

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

    const mapOptions: FileHashOptions<number> = {
      path,
      seed: 1,
      preferMap: true,
    };

    const blockOptions: FileHashOptions<number> = {
      path,
      seed: 1,
      preferMap: false,
    };

    bench
      .add(`map (${name})`, () => {
        xxhash3.file(mapOptions);
      })
      .add(`block (${name})`, () => {
        xxhash3.file(blockOptions);
      });
  }

  return bench;
}

export default { name, run };
