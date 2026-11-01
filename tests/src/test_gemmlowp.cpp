// gemmlowp (header-only): 8-bit matrix products against a plain C++
// reference -- odd sizes (33x29 times 29x47, which exercise the packing and
// kernel edges) with lhs/rhs offsets and an int32 result, single- and
// 4-threaded; the uint8 output pipeline (quantize down by scale, then
// saturating cast); and fixed-point exp/logistic on known values.
#include "common.h"
#include <gemmlowp/public/gemmlowp.h>
#include <gemmlowp/fixedpoint/fixedpoint.h>
#include <cmath>
#include <cstdint>
#include <vector>

using namespace gemmlowp;

int main() {
  QT_BEGIN("gemmlowp");
  const int M = 33, K = 29, N = 47, lo = -127, ro = -110;
  std::vector<std::uint8_t> a(M * K), b(K * N);
  for (int i = 0; i < M * K; i++) a[i] = (std::uint8_t)((i * 37 + 11) % 256);
  for (int i = 0; i < K * N; i++) b[i] = (std::uint8_t)((i * 53 + 7) % 256);
  std::vector<std::int32_t> ref(M * N);
  for (int i = 0; i < M; i++)
    for (int j = 0; j < N; j++) {
      std::int32_t s = 0;
      for (int k = 0; k < K; k++) s += (a[i * K + k] + lo) * (b[k * N + j] + ro);
      ref[i * N + j] = s;
    }
  MatrixMap<const std::uint8_t, MapOrder::RowMajor> lhs(a.data(), M, K);
  MatrixMap<const std::uint8_t, MapOrder::RowMajor> rhs(b.data(), K, N);
  for (int threads : {1, 4}) {
    GemmContext ctx; ctx.set_max_num_threads(threads);
    std::vector<std::int32_t> out(M * N, 0);
    MatrixMap<std::int32_t, MapOrder::RowMajor> res(out.data(), M, N);
    GemmWithOutputPipeline<std::uint8_t, std::int32_t, DefaultL8R8BitDepthParams>(&ctx, lhs, rhs, &res, lo, ro, std::make_tuple());
    QT_CHECK(out == ref, threads == 1 ? "33x29 * 29x47 with offsets, int32 result = reference (1 thread)"
                                      : "the same with 4 threads");
  }
  {
    GemmContext ctx;
    OutputStageQuantizeDownInt32ToUint8Scale q; q.result_offset = 1 << 17; q.result_mult_int = 3; q.result_shift = 12;
    OutputStageSaturatingCastToUint8 sat;
    std::vector<std::uint8_t> out(M * N, 0);
    MatrixMap<std::uint8_t, MapOrder::RowMajor> res(out.data(), M, N);
    GemmWithOutputPipeline<std::uint8_t, std::uint8_t, DefaultL8R8BitDepthParams>(&ctx, lhs, rhs, &res, lo, ro, std::make_tuple(q, sat));
    bool ok = true; int clamped = 0;
    for (int i = 0; i < M * N; i++) {
      std::int64_t v = ((std::int64_t)(ref[i] + q.result_offset) * q.result_mult_int + (1 << (q.result_shift - 1))) >> q.result_shift;
      if (v < 0 || v > 255) clamped++;
      v = v < 0 ? 0 : v > 255 ? 255 : v;
      ok &= out[i] == v;
    }
    printf("  info %d of %d outputs saturated\n", clamped, M * N);
    QT_CHECK(ok, "uint8 pipeline (quantize down by scale + saturating cast) = reference");
  }
  {
    typedef FixedPoint<std::int32_t, 5> F5; typedef FixedPoint<std::int32_t, 0> F0;
    double e = ToDouble(exp_on_negative_values(F5::FromDouble(-1.0)));
    double l = ToDouble(logistic(F5::FromDouble(0.0)));
    double t = ToDouble(tanh(F5::FromDouble(0.5)));
    printf("  info exp(-1) = %.7f, logistic(0) = %.7f, tanh(0.5) = %.7f\n", e, l, t);
    QT_CHECK(std::fabs(e - std::exp(-1.0)) < 1e-6, "fixed-point exp_on_negative_values(-1) = e^-1");
    QT_CHECK(std::fabs(l - 0.5) < 1e-6 && std::fabs(t - std::tanh(0.5)) < 1e-6, "fixed-point logistic(0) = 0.5, tanh(0.5)");
    QT_CHECK(ToDouble(F0::FromDouble(0.25) * F0::FromDouble(0.5)) == 0.125, "Q0.31 multiply 0.25 * 0.5 = 0.125");
  }
  QT_END();
}
