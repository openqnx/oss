/* Highway: portable SIMD. The dynamic-dispatch path picks a target for the
 * CPU actually running the test, which is the part a cross-build can break. */
#include "common.h"
#include <vector>
#include <hwy/highway.h>
#include <hwy/targets.h>
namespace hn = hwy::HWY_NAMESPACE;
int main() {
  QT_BEGIN("highway");
  printf("  info static target = %s\n", hwy::TargetName(HWY_STATIC_TARGET));
  QT_CHECK(hwy::SupportedTargets() != 0, "at least one SIMD target is supported");
  const hn::ScalableTag<float> d;
  const size_t N = hn::Lanes(d);
  QT_CHECK(N >= 1, "vector has at least one lane");
  printf("  info %zu float lanes\n", N);
  std::vector<float> a(N * 4), b(N * 4), out(N * 4);
  for (size_t i = 0; i < a.size(); ++i) { a[i] = float(i); b[i] = float(2 * i + 1); }
  for (size_t i = 0; i < a.size(); i += N) {
    const auto va = hn::Load(d, a.data() + i);
    const auto vb = hn::Load(d, b.data() + i);
    hn::Store(hn::MulAdd(va, vb, hn::Set(d, 1.0f)), d, out.data() + i);
  }
  bool ok = true;
  for (size_t i = 0; i < a.size(); ++i)
    if (out[i] != a[i] * b[i] + 1.0f) { ok = false; break; }
  QT_CHECK(ok, "MulAdd matches the scalar result for every lane");
  const hn::ScalableTag<int32_t> di;
  auto vi = hn::Iota(di, 1);
  QT_CHECK(hn::ReduceSum(di, vi) == int32_t(hn::Lanes(di) * (hn::Lanes(di) + 1) / 2),
           "ReduceSum of Iota matches the closed form");
  auto mask = hn::Gt(vi, hn::Set(di, 1));
  QT_CHECK(hn::CountTrue(di, mask) == hn::Lanes(di) - 1, "CountTrue on a comparison mask");
  QT_END();
}
