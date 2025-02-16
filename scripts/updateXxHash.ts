import path from 'path';
import fs from 'fs';
import { execFile } from 'child_process';

const scriptDirectory = import.meta.dirname;
const temporaryDirectory = path.join(scriptDirectory, '.temp');
const nativeDirectory = path.join(scriptDirectory, '../native');

function runProcess(command: string, args: string[]): Promise<string> {
  return new Promise<string>((resolve, reject) => {
    execFile(command, args, { env: process.env }, (error, stdout) => {
      if (error !== null) {
        reject(error);
      } else {
        resolve(stdout);
      }
    });
  });
}

async function cloneRepository() {
  await runProcess('git', [
    'clone',
    '--depth',
    '1',
    'https://github.com/Cyan4973/xxHash/',
    temporaryDirectory,
  ]);
}

async function copyFiles() {
  await fs.promises.cp(
    path.join(temporaryDirectory, 'xxhash.h'),
    path.join(nativeDirectory, 'xxhash.h'),
  );
}

async function updateLatestCommitFile() {
  process.chdir(temporaryDirectory);

  let commitHash = await runProcess('git', ['rev-parse', 'HEAD']);
  if (commitHash.endsWith('\n')) {
    commitHash = commitHash.slice(0, -1);
  }

  process.chdir(scriptDirectory);

  await fs.promises.writeFile(
    path.join(nativeDirectory, 'xxhash-commit.txt'),
    commitHash,
  );
}

async function cleanup() {
  await fs.promises.rm(temporaryDirectory, { force: true, recursive: true });
}

async function main() {
  await cloneRepository();
  await copyFiles();
  await updateLatestCommitFile();
  await cleanup();
}

void main();
