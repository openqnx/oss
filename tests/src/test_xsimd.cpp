/* xsimd (header-only): SIMD batches with known answers -- the architecture
 * compiled for (sse2 baseline on x86_64, NEON on aarch64), the CPU's
 * available instruction sets at run time, element-wise add/mul/fma,
 * horizontal sums, comparisons and select, and the vectorised math
 * functions (exp, log, sin, cos, sqrt) against <cmath> to a few ulps, over a
 * whole array processed batch by batch; int32 and double batches too. */
#include "common.h"
#include <xsimd/xsimd.hpp>
#include <cmath>
#include <cstdint>
#include <string>
#include <vector>

int main() {
  QT_BEGIN("xsimd");
  using bf = xsimd::batch<float>;
  using bd = xsimd::batch<double>;
  using bi = xsimd::batch<int32_t>;
  std::string arch = xsimd::default_arch::name();
  printf("  info xsimd %d.%d.%d, default arch %s, %zu floats per batch\n", XSIMD_VERSION_MAJOR, XSIMD_VERSION_MINOR,
         XSIMD_VERSION_PATCH, arch.c_str(), bf::size);
#if defined(__x86_64__)
  QT_CHECK(arch == "sse2" || arch.find("sse") == 0 || arch.find("avx") == 0, "x86_64: an SSE/AVX architecture");
  QT_CHECK(xsimd::available_architectures().sse2, "CPU reports SSE2 at run time");
#elif defined(__aarch64__)
  /* xsimd 14 names it "arm64+neon" (13 said "neon64") */
  QT_CHECK(arch.find("neon") != std::string::npos, "aarch64: a NEON architecture");
  QT_CHECK(xsimd::available_architectures().neon64, "CPU reports NEON at run time");
#endif
  QT_CHECK(bf::size >= 4 && bd::size >= 2, "batch widths >= 4 floats / 2 doubles");

  alignas(64) float a[16], b[16], r[16];
  for (int i = 0; i < 16; i++) { a[i] = float(i + 1); b[i] = float(2 * i); }
  bf x = bf::load_aligned(a), y = bf::load_aligned(b);
  (x + y).store_aligned(r);
  bool ok = true; for (size_t i = 0; i < bf::size; i++) ok &= r[i] == a[i] + b[i];
  QT_CHECK(ok, "add");
  xsimd::fma(x, y, bf(1.0f)).store_aligned(r);
  ok = true; for (size_t i = 0; i < bf::size; i++) ok &= r[i] == a[i] * b[i] + 1;
  QT_CHECK(ok, "fma(x, y, 1)");
  float hs = xsimd::reduce_add(x), want = 0; for (size_t i = 0; i < bf::size; i++) want += a[i];
  QT_CHECK(hs == want, "reduce_add = sum of lanes");
  QT_CHECK(xsimd::reduce_max(y) == b[bf::size - 1], "reduce_max");
  xsimd::select(x > bf(2.5f), x, bf(0.0f)).store_aligned(r);
  QT_CHECK(r[0] == 0 && r[1] == 0 && r[2] == 3, "select(x > 2.5, x, 0)");
  QT_CHECK(xsimd::any(x > bf(3.5f)) && !xsimd::all(x > bf(3.5f)), "any / all of a mask");

  /* math over 1024 values, batch by batch, against <cmath> */
  const size_t N = 1024;
  std::vector<float> in(N), oe(N), ol(N), os(N), oc(N), oq(N);
  for (size_t i = 0; i < N; i++) in[i] = 0.01f + 10.0f * float(i) / N;
  for (size_t i = 0; i < N; i += bf::size) {
    bf v = bf::load_unaligned(&in[i]);
    xsimd::exp(v).store_unaligned(&oe[i]); xsimd::log(v).store_unaligned(&ol[i]);
    xsimd::sin(v).store_unaligned(&os[i]); xsimd::cos(v).store_unaligned(&oc[i]);
    xsimd::sqrt(v).store_unaligned(&oq[i]);
  }
  auto rel = [](float g, float w) { return std::fabs(g - w) / std::fmax(std::fabs(w), 1e-6f); };
  double me = 0, ml = 0, ms = 0, mc = 0, mq = 0;
  for (size_t i = 0; i < N; i++) {
    me = std::fmax(me, rel(oe[i], std::exp(in[i]))); ml = std::fmax(ml, rel(ol[i], std::log(in[i])));
    ms = std::fmax(ms, std::fabs(os[i] - std::sin(in[i]))); mc = std::fmax(mc, std::fabs(oc[i] - std::cos(in[i])));
    mq = std::fmax(mq, rel(oq[i], std::sqrt(in[i])));
  }
  printf("  info max error: exp %.2g log %.2g sin %.2g cos %.2g sqrt %.2g\n", me, ml, ms, mc, mq);
  QT_CHECK(me < 1e-6 && ml < 1e-6, "exp, log within 1e-6 relative");
  QT_CHECK(ms < 1e-6 && mc < 1e-6, "sin, cos within 1e-6 absolute");
  QT_CHECK(mq < 2.5e-7, "sqrt within 2 ulp");

  alignas(64) double dd[8] = {1, 4, 9, 16, 25, 36, 49, 64}, dr[8];
  xsimd::sqrt(bd::load_aligned(dd)).store_aligned(dr);
  ok = true; for (size_t i = 0; i < bd::size; i++) ok &= dr[i] == double(i + 1);
  QT_CHECK(ok, "double sqrt of squares is exact");
  alignas(64) int32_t ia[16], ir[16];
  for (int i = 0; i < 16; i++) ia[i] = i - 8;
  xsimd::abs(bi::load_aligned(ia) * bi(3)).store_aligned(ir);
  ok = true; for (size_t i = 0; i < bi::size; i++) ok &= ir[i] == std::abs((int(i) - 8) * 3);
  QT_CHECK(ok, "int32 multiply and abs");
  QT_END();
}
