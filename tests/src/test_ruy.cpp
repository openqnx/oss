// ruy (TensorFlow Lite's matrix multiplication library, static libraries):
// float and 8-bit quantized products with known results, a larger product
// against a naive loop on several threads, row/column-major layouts, bias
// and clamping in the multiplication parameters, and which kernel path the
// CPU got (AVX2/AVX-512 on x86_64, NEON on aarch64).
#include "common.h"
#include <ruy/ruy.h>
#include <cmath>
#include <cstdint>
#include <vector>

template <typename T>
static void make(ruy::Matrix<T> *m, int rows, int cols, ruy::Order order, T *data) {
  ruy::MakeSimpleLayout(rows, cols, order, m->mutable_layout());
  m->set_data(data);
}

// The reference product and the error reduction are kept out of the
// vectoriser: the SDP's GCC for aarch64 hits an internal compiler error
// (vect_transform_reduction) on the max-|error| loop.
__attribute__((noinline, optimize("no-tree-vectorize")))
static void naive(const float *x, const float *y, float *ref, int M, int K, int N) {
  for (int i = 0; i < M; i++) for (int k = 0; k < K; k++) for (int j = 0; j < N; j++) ref[i * N + j] += x[i * K + k] * y[k * N + j];
}
__attribute__((noinline, optimize("no-tree-vectorize")))
static double max_error(const float *a, const float *b, int n) {
  double m = 0;
  for (int i = 0; i < n; i++) { double d = std::fabs((double)a[i] - b[i]); if (d > m) m = d; }
  return m;
}

int main() {
  QT_BEGIN("ruy");
  ruy::Context ctx;
  // [1 2; 3 4] x [5 6; 7 8] = [19 22; 43 50]
  float a[] = { 1, 2, 3, 4 }, b[] = { 5, 6, 7, 8 }, c[4] = { 0 };
  ruy::Matrix<float> A, B, C;
  make(&A, 2, 2, ruy::Order::kRowMajor, a);
  make(&B, 2, 2, ruy::Order::kRowMajor, b);
  make(&C, 2, 2, ruy::Order::kRowMajor, c);
  ruy::MulParams<float, float> mp;
  ruy::Mul(A, B, mp, &ctx, &C);
  QT_CHECK(c[0] == 19 && c[1] == 22 && c[2] == 43 && c[3] == 50, "float 2x2: [19 22; 43 50]");
  // column-major B is B transposed in memory: same product
  float bt[] = { 5, 7, 6, 8 }, c2[4] = { 0 };
  ruy::Matrix<float> Bc, C2;
  make(&Bc, 2, 2, ruy::Order::kColMajor, bt);
  make(&C2, 2, 2, ruy::Order::kRowMajor, c2);
  ruy::Mul(A, Bc, mp, &ctx, &C2);
  QT_CHECK(c2[0] == 19 && c2[3] == 50, "column-major operand, same result");
  // bias and clamp
  float bias[] = { 1, -100 }, c3[4] = { 0 };
  ruy::MulParams<float, float> mpb; mpb.set_bias(bias); mpb.set_clamp_max(40.f);
  ruy::Matrix<float> C3; make(&C3, 2, 2, ruy::Order::kRowMajor, c3);
  ruy::Mul(A, B, mpb, &ctx, &C3);
  QT_CHECK(c3[0] == 20 && c3[1] == 23 && c3[2] == -57 && c3[3] == -50, "per-row bias (+1, -100)");
  float c4[4]; ruy::Matrix<float> C4; make(&C4, 2, 2, ruy::Order::kRowMajor, c4);
  ruy::MulParams<float, float> mpc; mpc.set_clamp_max(40.f); mpc.set_clamp_min(20.f);
  ruy::Mul(A, B, mpc, &ctx, &C4);
  QT_CHECK(c4[0] == 20 && c4[1] == 22 && c4[2] == 40 && c4[3] == 40, "clamp to [20, 40]");

  // int8 x int8 -> int32 raw accumulators
  int8_t qa[] = { 1, -2, 3, 4, 5, -6 }, qb[] = { 7, 8, -9, 10, 11, 12 };   // 2x3 x 3x2
  int32_t qc[4];
  ruy::Matrix<int8_t> QA, QB; ruy::Matrix<int32_t> QC;
  make(&QA, 2, 3, ruy::Order::kRowMajor, qa);
  make(&QB, 3, 2, ruy::Order::kRowMajor, qb);
  make(&QC, 2, 2, ruy::Order::kRowMajor, qc);
  ruy::MulParams<int32_t, int32_t> qmp;
  ruy::Mul(QA, QB, qmp, &ctx, &QC);
  QT_CHECK(qc[0] == 7 + 18 + 33 && qc[1] == 8 - 20 + 36 && qc[2] == 28 - 45 - 66 && qc[3] == 32 + 50 - 72,
           "int8 2x3 x 3x2 -> int32 [58 24; -83 10]");
  // uint8 with zero points, requantised to uint8: (a-128)(b-128) * 0.5 + 128
  uint8_t ua[] = { 130, 126 }, ub[] = { 132, 124 };   // 1x2 x 2x1 : (2*4 + -2*-4) = 16
  uint8_t uc[1];
  ruy::Matrix<uint8_t> UA, UB, UC;
  make(&UA, 1, 2, ruy::Order::kRowMajor, ua); UA.set_zero_point(128);
  make(&UB, 2, 1, ruy::Order::kColMajor, ub); UB.set_zero_point(128);
  make(&UC, 1, 1, ruy::Order::kColMajor, uc); UC.set_zero_point(128);
  ruy::MulParams<int32_t, uint8_t> ump; ump.set_multiplier_fixedpoint(1 << 30); ump.set_multiplier_exponent(0);   // x 0.5
  ruy::Mul(UA, UB, ump, &ctx, &UC);
  QT_CHECK(uc[0] == 128 + 8, "uint8 with zero points 128, multiplier 0.5: 136");

  // 96x80 x 80x72 on 2 threads against a naive loop
  const int M = 96, K = 80, N = 72;
  std::vector<float> x(M * K), y(K * N), z(M * N), ref(M * N, 0.f);
  for (int i = 0; i < M * K; i++) x[i] = (float)((i * 7) % 13) - 6.f;
  for (int i = 0; i < K * N; i++) y[i] = (float)((i * 5) % 11) * 0.25f - 1.f;
  naive(x.data(), y.data(), ref.data(), M, K, N);
  ctx.set_max_num_threads(2);
  ruy::Matrix<float> X, Y, Z;
  make(&X, M, K, ruy::Order::kRowMajor, x.data());
  make(&Y, K, N, ruy::Order::kRowMajor, y.data());
  make(&Z, M, N, ruy::Order::kRowMajor, z.data());
  ruy::Mul(X, Y, mp, &ctx, &Z);
  double maxerr = max_error(z.data(), ref.data(), M * N);
  printf("  info 96x80x72 max |error| %g\n", maxerr);
  QT_CHECK(maxerr < 1e-3, "96x80 x 80x72 float product on 2 threads matches the naive loop");
  ruy::Path p = ctx.last_used_path();
  printf("  info kernel path 0x%x\n", (unsigned)p);
#if defined(__x86_64__)
  QT_CHECK(p != ruy::Path::kNone, "a kernel path was selected");
#elif defined(__aarch64__)
  QT_CHECK(p == ruy::Path::kNeon || p == ruy::Path::kNeonDotprod, "NEON kernel path on aarch64");
#endif
  QT_END();
}
