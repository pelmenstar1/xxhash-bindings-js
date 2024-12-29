import { Bench } from 'tinybench';
import fs from 'fs';
import { generateRandomFileContent } from './randomDataGenerator.ts';

import { MB, TEST_DATA_PATH } from './constants.ts';
import { allnative, DirectoryHashingOptions, minimum } from './libs.ts';

export const name = 'directory';

async function generateRandomDirectoryContent(
  dirPath: string,
  fileCount: number,
  fileSize: number,
): Promise<void> {
  await Promise.all(
    [...Array(fileCount).keys()].map(async (index) => {
      const filePath = `${dirPath}/${index + 1}`;

      if (!fs.existsSync(filePath)) {
        await generateRandomFileContent(filePath, fileSize);
      }
    }),
  );
}

export async function run(): Promise<Bench> {
  const dirInfos: { name: string; count: number; fileSize: number }[] = [
    { name: 'dir1', count: 1, fileSize: 10 * MB },
    { name: 'dir10', count: 10, fileSize: 50 * MB },
    { name: 'dir100', count: 100, fileSize: 5 * MB },
    { name: 'dir500', count: 500, fileSize: MB },
  ];

  console.log('Checking data files');

  await Promise.all(
    dirInfos.map(({ name }) =>
      fs.promises.mkdir(`${TEST_DATA_PATH}/${name}`, { recursive: true }),
    ),
  );

  await Promise.all(
    dirInfos.map(({ name, count, fileSize }) =>
      generateRandomDirectoryContent(
        `${TEST_DATA_PATH}/${name}`,
        count,
        fileSize,
      ),
    ),
  );

  console.log('Starting benchmark');

  const bench = new Bench({
    warmupIterations: 3,
    iterations: 15,
  });

  for (const { name } of dirInfos) {
    const path = `${TEST_DATA_PATH}/${name}`;

    const mapOptions: DirectoryHashingOptions<number> = {
      path,
      seed: 1,
      preferMap: true,
    };

    const blockOptions: DirectoryHashingOptions<number> = {
      path,
      seed: 1,
      preferMap: false,
    };

    bench
      .add(`all map (${name})`, () => {
        allnative.xxhash3.directory(mapOptions);
      })
      .add(`all block (${name})`, () => {
        allnative.xxhash3.directory(blockOptions);
      })
      .add(`min block (${name})`, () => {
        minimum.xxhash3.directory(blockOptions);
      });
  }

  return bench;
}

export default { name, run };
