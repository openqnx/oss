/* oneTBB: parallel_reduce / parallel_for / parallel_sort / parallel_scan
 * against closed forms, concurrent_hash_map counting under contention,
 * task_group, a flow graph pipeline, and the scalable allocator. */
#include "common.h"
#include <oneapi/tbb.h>
#include <oneapi/tbb/scalable_allocator.h>
#include <atomic>
#include <vector>
#include <cstdint>
int main() {
  QT_BEGIN("onetbb");
  printf("  info TBB %s, interface %d, max concurrency %d\n", TBB_runtime_version(), TBB_runtime_interface_version(),
         tbb::this_task_arena::max_concurrency());
  QT_CHECK(TBB_runtime_interface_version() == TBB_INTERFACE_VERSION, "runtime interface version matches header");
  QT_CHECK(tbb::this_task_arena::max_concurrency() >= 1, "task arena has workers");

  const int64_t N = 1000000;
  int64_t sum = tbb::parallel_reduce(tbb::blocked_range<int64_t>(1, N + 1), (int64_t)0,
      [](const tbb::blocked_range<int64_t> &r, int64_t acc) { for (int64_t i = r.begin(); i != r.end(); ++i) acc += i * i; return acc; },
      std::plus<int64_t>());
  int64_t want = N * (N + 1) * (2 * N + 1) / 6;
  printf("  info sum of squares 1..%lld = %lld\n", (long long)N, (long long)sum);
  QT_CHECK(sum == want, "parallel_reduce: sum of squares = n(n+1)(2n+1)/6");

  std::vector<int> v(200000, 0);
  tbb::parallel_for(tbb::blocked_range<size_t>(0, v.size()),
      [&](const tbb::blocked_range<size_t> &r) { for (size_t i = r.begin(); i != r.end(); ++i) v[i] += (int)(i % 7) + 1; });
  bool ok = true;
  for (size_t i = 0; i < v.size(); i++) ok &= v[i] == (int)(i % 7) + 1;
  QT_CHECK(ok, "parallel_for: every element written exactly once");

  std::vector<uint32_t> s(100000);
  uint32_t x = 12345;
  for (auto &e : s) { x = x * 1664525u + 1013904223u; e = x; }
  tbb::parallel_sort(s.begin(), s.end());
  ok = true;
  for (size_t i = 1; i < s.size(); i++) ok &= s[i - 1] <= s[i];
  QT_CHECK(ok, "parallel_sort: 100000 LCG values sorted");

  std::vector<int64_t> pre(10000);
  tbb::parallel_scan(tbb::blocked_range<size_t>(0, pre.size()), (int64_t)0,
      [&](const tbb::blocked_range<size_t> &r, int64_t run, bool final) {
        for (size_t i = r.begin(); i != r.end(); ++i) { run += (int64_t)i + 1; if (final) pre[i] = run; }
        return run; }, std::plus<int64_t>());
  ok = true;
  for (size_t i = 0; i < pre.size(); i++) ok &= pre[i] == (int64_t)(i + 1) * (int64_t)(i + 2) / 2;
  QT_CHECK(ok, "parallel_scan: prefix sums are triangular numbers");

  typedef tbb::concurrent_hash_map<int, int> Map;
  Map m;
  tbb::parallel_for(0, 100000, [&](int i) { Map::accessor a; m.insert(a, i % 100); a->second += 1; });
  ok = m.size() == 100;
  for (int k = 0; k < 100; k++) { Map::const_accessor a; ok &= m.find(a, k) && a->second == 1000; }
  QT_CHECK(ok, "concurrent_hash_map: 100 keys x 1000 increments each");

  std::atomic<int> hits{0};
  tbb::task_group g;
  for (int i = 0; i < 64; i++) g.run([&hits, i] { hits += i; });
  g.wait();
  QT_CHECK(hits == 64 * 63 / 2, "task_group: 64 tasks ran");

  /* flow graph: source numbers -> square -> sum */
  tbb::flow::graph fg;
  std::atomic<long> fsum{0};
  tbb::flow::function_node<int, int> sq(fg, tbb::flow::unlimited, [](int i) { return i * i; });
  tbb::flow::function_node<int> acc(fg, 1, [&](int i) { fsum += i; return tbb::flow::continue_msg(); });
  tbb::flow::make_edge(sq, acc);
  for (int i = 1; i <= 100; i++) sq.try_put(i);
  fg.wait_for_all();
  QT_CHECK(fsum == 338350, "flow graph: sum of squares 1..100 = 338350");

  void *p = scalable_malloc(1 << 20);
  QT_CHECK(p != nullptr, "scalable_malloc 1 MiB");
  if (p) { memset(p, 0xa5, 1 << 20); QT_CHECK(((unsigned char *)p)[(1 << 20) - 1] == 0xa5, "scalable memory usable"); scalable_free(p); }
  QT_END();
}
