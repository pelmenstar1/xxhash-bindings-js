import { spawn } from 'child_process';
import fs from 'fs';

function runCommand(command: string, args: string[]): Promise<void> {
  return new Promise((resolve, reject) => {
    const proc = spawn(command, args, { shell: true });
    proc.stdout.pipe(process.stdout);
    proc.stderr.pipe(process.stderr);

    proc.on('close', (code) => {
      if (code === 0) {
        resolve(undefined);
      } else {
        reject(new Error(`Process ${command} returned with code ${code}`));
      }
    });
  });
}

function parseOptions(): Set<string> {
  const result = new Set<string>();

  for (const arg of process.argv.slice(2)) {
    if (arg.startsWith('--')) {
      result.add(arg.slice(2));
    }
  }

  return result;
}

async function main() {
  const options = parseOptions();
  const debug = options.has('debug');

  process.chdir('../library');

  if (!options.has('no-conf')) {
    await runCommand('yarn', [
      'node-gyp',
      'configure',
      ...(debug ? ['--debug'] : []),
    ]);
  }

  await runCommand('yarn', ['node-gyp', 'build', '-j', 'max']);

  await fs.promises.mkdir('./dist', { recursive: true });
  await Promise.all([
    fs.promises.cp(
      `./build/${debug ? 'Debug' : 'Release'}/xxhash.node`,
      `./dist/xxhash-${process.platform}-${process.arch}.node`,
    ),
    fs.promises.cp('./index.js', './dist/index.js'),
    fs.promises.cp('./index.d.ts', './dist/index.d.ts'),
  ]);

  console.log('Finished successfully');
}

void main();
