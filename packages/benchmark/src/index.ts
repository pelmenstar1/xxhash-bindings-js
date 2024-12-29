import { Bench } from 'tinybench';
import process from 'process';

type BenchmarkModule = {
  name: string;
  run: () => Promise<Bench>;
};

const benchmarksPath: string[] = ['fileBenchmark', 'directoryBenchmark'];

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
  const benchmarks: BenchmarkModule[] = await Promise.all(
    benchmarksPath.map((path) => import(`./${path}.ts`)),
  );

  const module = benchmarks.find(({ name }) => name === specBenchName);

  if (module === undefined) {
    console.log('Cannot find benchmark with given name');
  } else {
    await runBenchmark(module);
  }
}

void main();
