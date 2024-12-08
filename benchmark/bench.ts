import { FileHashingOptions, FileHashingType, xxhash3 } from '..';

import { Bench } from 'tinybench';

import fs from 'fs';
import { generateRandomFileContent } from './randomDataGenerator';

const DATA_FILE_NAME = "gb1";
const DATA_FILE_PATH = `./.data/${DATA_FILE_NAME}`;

const mapOptions: FileHashingOptions<number> = {
  path: DATA_FILE_PATH,
  seed: 1
};

const blockOptions: FileHashingOptions<number> = {
  path: DATA_FILE_PATH,
  seed: 1,
  type: FileHashingType.BLOCK
};  

async function main() {
  if (!fs.existsSync(DATA_FILE_PATH)) {
    await generateRandomFileContent(DATA_FILE_PATH, 1024 * 1024 * 1024);
  }

  const bench = new Bench({ name: 'file hashing', warmupIterations: 5, iterations: 15 });

  bench
    .add('map', () => {
      xxhash3.file(mapOptions);
    })
    .add('block', () => {
      xxhash3.file(blockOptions);
    });

  await bench.run();

  console.log(bench.name);
  console.table(bench.table());
}

void main();
