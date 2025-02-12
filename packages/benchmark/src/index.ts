import { Bench } from 'tinybench';
import process from 'process';

type BenchmarkModule = {
  name: string;
  run: () => Promise<Bench>;
};

async function runBenchmark(module: BenchmarkModule) {
  const bench = await module.run();
  bench.addEventListener('cycle', ({ task }) => {
    if (task != undefined) {
      console.log(
        `${task.name} (${task.runs}): time = ${task.result.period} ms`,
      );
    }
  });

  await bench.run();

  console.log();
  console.table(bench.table());
}

async function main() {
  const { argv } = process;

  if (argv.length <= 2) {
    console.log('Invalid command. Specify the benchmark name');
  }

  const specBenchName = argv[2];
  const module: BenchmarkModule = await import(`./${specBenchName}.ts`);

  await runBenchmark(module);
}

void main();
