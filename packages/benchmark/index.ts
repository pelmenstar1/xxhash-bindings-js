import type { FileHashingOptions } from 'xxhash-bindings-min';
import { xxhash3 as minXxHash3 } from 'xxhash-bindings-min';
import { xxhash3 as allXxHash3 } from 'xxhash-bindings-allnative';

import { Bench } from 'tinybench';

import fs from 'fs';
import { generateRandomFileContent } from './randomDataGenerator';

const DATA_FILE_PATH = `./.data/gb1`;

const mapOptions: FileHashingOptions<number> = {
  path: DATA_FILE_PATH,
  seed: 1,
  preferMap: true,
};

const blockOptions: FileHashingOptions<number> = {
  path: DATA_FILE_PATH,
  seed: 1,
  preferMap: false,
};

async function main() {
  console.log('Checking data file');

  if (!fs.existsSync(DATA_FILE_PATH)) {
    generateRandomFileContent(DATA_FILE_PATH, 1024 * 1024 * 1024);
  }

  console.log('Starting benchmark');

  const bench = new Bench({
    name: 'file hashing',
    warmupIterations: 5,
    iterations: 15,
  });

  bench.addEventListener('cycle', ({ task }) => {
    if (task != undefined) {
      console.log(
        `cycle ${task.name} (${task.runs}): time = ${task.result.period} ms`,
      );
    }
  });

  bench.addEventListener('warmup', ({ task }) => {
    if (task != undefined) {
      console.log(
        `warmup ${task.name} (${task.runs}): time = ${task.result.period} ms`,
      );
    }
  });

  bench
    .add('all map', () => {
      allXxHash3.file(mapOptions);
    })
    .add('all block', () => {
      allXxHash3.file(blockOptions);
    })
    .add('min block', () => {
      minXxHash3.file(blockOptions);
    });

  await bench.run();

  console.log(bench.name);
  console.table(bench.table());
}

void main();
