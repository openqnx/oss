/* openmp (libomp): OpenMP programs compiled with the SDP's GCC (-fopenmp)
 * and linked against LLVM's libomp, which implements GCC's GOMP_* entry
 * points (the SDP has no libgomp).
 *
 * A parallel-for reduction over 1..N has the known sum; a team of 4 threads
 * has 4 distinct thread numbers; an atomic counter and a dynamic schedule
 * count every iteration exactly once; tasks compute fib(20) = 6765; a
 * critical section and a barrier work; and the runtime API (omp_*) agrees
 * with what the pragmas did. */
#include "common.h"
#include <omp.h>

static long fib(int n) {
  if (n < 2) return n;
  long a, b;
#pragma omp task shared(a) if (n > 12)
  a = fib(n - 1);
#pragma omp task shared(b) if (n > 12)
  b = fib(n - 2);
#pragma omp taskwait
  return a + b;
}

int main(void) {
  QT_BEGIN("openmp");
  printf("  info omp_get_num_procs()=%d omp_get_max_threads()=%d _OPENMP=%d\n",
         omp_get_num_procs(), omp_get_max_threads(), _OPENMP);
  QT_CHECK(omp_get_num_procs() >= 1 && omp_get_max_threads() >= 1, "the runtime sees >= 1 processor");
  QT_CHECK(!omp_in_parallel() && omp_get_thread_num() == 0, "serial part: not in parallel, thread 0");

  const long N = 1000000;
  long sum = 0;
#pragma omp parallel for reduction(+ : sum) num_threads(4)
  for (long i = 1; i <= N; i++) sum += i;
  QT_CHECK(sum == 500000500000L, "parallel for reduction(+) over 1..1000000 = 500000500000");

  int nthreads = 0, mask = 0, inpar = 1;
#pragma omp parallel num_threads(4)
  {
#pragma omp single
    nthreads = omp_get_num_threads();
#pragma omp critical
    mask |= 1 << omp_get_thread_num();
    if (!omp_in_parallel()) {
#pragma omp atomic write
      inpar = 0;
    }
#pragma omp barrier
  }
  QT_CHECK(nthreads == 4, "a team of num_threads(4) has 4 threads");
  QT_CHECK(mask == 0xF, "thread numbers 0..3, each once (critical section)");
  QT_CHECK(inpar, "omp_in_parallel() inside the region");

  int count = 0, hits[1000] = {0};
#pragma omp parallel for schedule(dynamic, 7) num_threads(4)
  for (int i = 0; i < 1000; i++) {
#pragma omp atomic
    count++;
    hits[i]++;
  }
  int once = 1;
  for (int i = 0; i < 1000; i++) once &= hits[i] == 1;
  QT_CHECK(count == 1000 && once, "schedule(dynamic,7): 1000 iterations, each exactly once (atomic)");

  long f = 0;
#pragma omp parallel num_threads(4)
#pragma omp single
  f = fib(20);
  QT_CHECK(f == 6765, "tasks: fib(20) = 6765");

  double t0 = omp_get_wtime();
  QT_CHECK(t0 > 0 && omp_get_wtime() >= t0 && omp_get_wtick() > 0, "omp_get_wtime / omp_get_wtick");
  omp_set_num_threads(3);
  int n3 = 0;
#pragma omp parallel
#pragma omp single
  n3 = omp_get_num_threads();
  QT_CHECK(n3 == 3, "omp_set_num_threads(3) gives a team of 3");
  QT_END();
}
