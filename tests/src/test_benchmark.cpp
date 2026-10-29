// Google Benchmark: register and run one benchmark.
#include "common.h"
#include <benchmark/benchmark.h>
static int runs = 0;
static void BM_t(benchmark::State &s) { for (auto _ : s) benchmark::DoNotOptimize(runs++); }
BENCHMARK(BM_t)->Iterations(1000);
int main() {
  QT_BEGIN("benchmark");
  int argc = 1; char a0[] = "t"; char *argv[] = {a0, nullptr};
  benchmark::Initialize(&argc, argv);
  size_t n = benchmark::RunSpecifiedBenchmarks();
  QT_CHECK(n == 1, "one benchmark ran");
  QT_CHECK(runs >= 1000, "iterations executed");
  benchmark::Shutdown();
  QT_END();
}
