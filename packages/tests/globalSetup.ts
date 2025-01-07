import { TEST_DATA_PATH } from './utils';
import { promises as fs } from 'fs';

async function setupDirectories() {
  await Promise.all(
    ['dir', 'empty_dir', 'dir/empty_dir', 'dir/dir2'].map((p) =>
      fs.mkdir(`${TEST_DATA_PATH}/${p}`, { recursive: true }),
    ),
  );

  await Promise.all(
    [
      ['file1.txt', 'content'],
      ['file2.txt', 'content 2'],
      ['dir2/file3.txt', 'content 3'],
    ].map(([p, content]) =>
      fs.writeFile(`${TEST_DATA_PATH}/dir/${p}`, content, { encoding: 'utf8' }),
    ),
  );
}

export async function setup() {
  await setupDirectories();
}
