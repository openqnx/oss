/* pthreadpool: a 4-thread pool; parallelize_1d computes a known sum with
 * atomics, 2d and 2d_tile_2d cover every (i,j) of a non-multiple-of-tile
 * grid exactly once, 1d_tile_1d tiles add up, and a NULL pool runs the
 * same work serially on the caller. */
#include "common.h"
#include <stdatomic.h>
#include <stdint.h>
#include <stdlib.h>
#include <pthreadpool.h>
static void sum1d(void *ctx, size_t i) { atomic_fetch_add((_Atomic uint64_t *)ctx, (uint64_t)i * i); }
#define RI 37
#define RJ 53
static _Atomic int grid[RI][RJ];
static void cell(void *ctx, size_t i, size_t j) { (void)ctx; atomic_fetch_add(&grid[i][j], 1); }
static void tile(void *ctx, size_t i, size_t j, size_t ti, size_t tj) {
  (void)ctx; for (size_t a = i; a < i + ti; a++) for (size_t b = j; b < j + tj; b++) atomic_fetch_add(&grid[a][b], 1);
}
static void tile1(void *ctx, size_t start, size_t n) { atomic_fetch_add((_Atomic uint64_t *)ctx, (uint64_t)n); (void)start; }
static void thr(void *ctx, size_t t, size_t i) { (void)i; atomic_fetch_or((_Atomic uint64_t *)ctx, (uint64_t)1 << t); }
static int grid_all(int v) {
  for (int i = 0; i < RI; i++) for (int j = 0; j < RJ; j++) if (grid[i][j] != v) return 0;
  return 1;
}
int main(void) {
  QT_BEGIN("pthreadpool");
  pthreadpool_t p0 = pthreadpool_create(0);
  QT_CHECK(p0 != NULL, "pthreadpool_create(0): one thread per CPU");
  if (p0) { printf("  info default pool: %zu threads\n", pthreadpool_get_threads_count(p0)); pthreadpool_destroy(p0); }
  pthreadpool_t p = pthreadpool_create(4);
  QT_CHECK(p != NULL && pthreadpool_get_threads_count(p) == 4, "4-thread pool");
  if (!p) QT_END();
  _Atomic uint64_t s = 0;
  pthreadpool_parallelize_1d(p, sum1d, (void *)&s, 100000, 0);
  QT_CHECK(s == (uint64_t)99999 * 100000 * 199999 / 6, "parallelize_1d: sum of i^2 for i < 100000");
  pthreadpool_parallelize_2d(p, cell, NULL, RI, RJ, 0);
  QT_CHECK(grid_all(1), "parallelize_2d: every cell of 37x53 visited once");
  pthreadpool_parallelize_2d_tile_2d(p, tile, NULL, RI, RJ, 8, 16, 0);
  QT_CHECK(grid_all(2), "parallelize_2d_tile_2d (8x16 tiles): every cell once more");
  s = 0;
  pthreadpool_parallelize_1d_tile_1d(p, tile1, (void *)&s, 1001, 64, 0);
  QT_CHECK(s == 1001, "parallelize_1d_tile_1d: tile sizes add up to the range");
  _Atomic uint64_t mask = 0;
  pthreadpool_parallelize_1d_with_thread(p, thr, (void *)&mask, 4096, 0);
  printf("  info thread mask %llx\n", (unsigned long long)mask);
  QT_CHECK(mask != 0 && (mask & ~(uint64_t)0xf) == 0, "thread ids are within 0..3");
  s = 0;
  pthreadpool_parallelize_1d(NULL, sum1d, (void *)&s, 1000, 0);
  QT_CHECK(s == (uint64_t)999 * 1000 * 1999 / 6, "NULL pool runs serially");
  pthreadpool_destroy(p);
  QT_END();
}
