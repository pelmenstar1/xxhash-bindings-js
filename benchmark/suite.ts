import { hrtime } from "node:process";
import { tableBuilder } from "./table";

type Benchmark = {
  name: string;
  worker: () => unknown;
}

type BenchmarkStats = {
  min: number,
  max: number,
  average: number;
  median: number;
}

const benchmarks: Benchmark[] = [];

function computeStats(data: bigint[]): BenchmarkStats {
  const numberData = data.map(Number);

  let sum = 0;
  let min = Number.MAX_VALUE;
  let max = -1;

  for (const value of numberData) {
    if (value < min) {
      min = value;
    }

    if (value > max) {
      max = value;
    }

    sum += value;
  }

  const average = sum / data.length;
  const sorted = numberData.sort((a, b) => a - b);

  const mid = Math.floor(data.length / 2);
  let median: number;

  if (mid * 2 == data.length) {
    median = (sorted[mid] + sorted[mid + 1]) * 0.5;
  } else {
    median = sorted[mid];
  }

  return { min, max, average, median };
}

function formatNanoseconds(value: number): string {
  if (value < 1000) {
    return `${value.toFixed(2)} ns`
  }

  if (value < 1_000_000) {
    return `${(value / 1000).toFixed(2)} μs`
  }

  if (value < 1_000_000_000) {
    return `${(value / 1_000_000).toFixed(2)} ms`
  }

  return `${(value / 1_000_000_000).toFixed(2)} s`
}

function printStats({ min, max, average, median }: BenchmarkStats) {
  console.log(`Minimum time: ${formatNanoseconds(min)}`)
  console.log(`Maximum time: ${formatNanoseconds(max)}`)
  console.log(`Average: ${formatNanoseconds(average)}`)
  console.log(`Median: ${formatNanoseconds(median)}`)
}

export function benchmark(value: Benchmark) {
  benchmarks.push(value);
}

export function runBenchmark(benchmark: Benchmark): BenchmarkStats {
  function warmUp() {
    const { worker } = benchmark;

    for (let i = 0; i < 4; i++) {
      worker();
    }
  }

  function workload(): bigint[] {
    const ITERATION_COUNT = 16;

    const elapsedTimes: bigint[] = [];
    const { worker } = benchmark;

    for (let i = 0; i < ITERATION_COUNT; i++) {
      const startTime = hrtime.bigint();
      worker();
      const endTime = hrtime.bigint();

      const elapsed = endTime - startTime;
      elapsedTimes.push(elapsed);

      console.log(`Iteration ${i + 1}: time = ${elapsed} ns`)
    }

    return elapsedTimes;
  }

  console.log(`Running '${benchmark.name}' benchmark`);
  console.log("Warming up...")
  warmUp();

  console.log("Workload...");
  const results = workload();

  return computeStats(results);
}

function printMergedStatsTable(benchmarks: Benchmark[], statsList: BenchmarkStats[]) {
  const builder = tableBuilder(["Benchmark", "Min", "Max", "Avg", "Median"]);
  for (let i = 0; i < statsList.length; i++) {
    const benchmark = benchmarks[i];
    const stats = statsList[i];

    builder.append([
      benchmark.name,
      formatNanoseconds(stats.min),
      formatNanoseconds(stats.max),
      formatNanoseconds(stats.average),
      formatNanoseconds(stats.median)
    ]);
  }

  console.log();
  builder.create().print();
}

export function runAll() {
  const statsList = benchmarks.map(runBenchmark);

  printMergedStatsTable(benchmarks, statsList);
}