/* fftw: FFTs of signals whose spectra are known exactly -- a cosine (two
 * spikes of N/2 at bins k and N-k), a unit impulse (flat spectrum), a
 * constant (all energy in bin 0) through the real-to-complex transform,
 * a DCT-II of a constant, a 2-D transform; forward + backward = N * input;
 * a non-power-of-two length (1000 = 2^3 5^3) against a naive O(N^2) DFT;
 * the same in single precision (libfftw3f, NEON/SSE/AVX codelets); and a
 * multi-threaded plan (libfftw3_threads) giving the same answer. */
#include "common.h"
#include <math.h>
#include <stdlib.h>
#include <fftw3.h>

static double maxerr(fftw_complex *a, fftw_complex *b, int n) {
  double e = 0;
  for (int i = 0; i < n; i++) {
    double d = hypot(a[i][0] - b[i][0], a[i][1] - b[i][1]);
    if (d > e) e = d;
  }
  return e;
}

int main(void) {
  QT_BEGIN("fftw");
  printf("  info %s\n", fftw_version);
  const double PI = 3.14159265358979323846;
  int N = 64;
  fftw_complex *in = fftw_alloc_complex(N), *out = fftw_alloc_complex(N), *back = fftw_alloc_complex(N);
  fftw_plan p = fftw_plan_dft_1d(N, in, out, FFTW_FORWARD, FFTW_ESTIMATE);
  fftw_plan q = fftw_plan_dft_1d(N, out, back, FFTW_BACKWARD, FFTW_ESTIMATE);

  /* cos(2 pi 5 n / N): X[5] = X[59] = N/2, all other bins 0 */
  for (int n = 0; n < N; n++) { in[n][0] = cos(2 * PI * 5 * n / N); in[n][1] = 0; }
  fftw_execute(p);
  double rest = 0;
  for (int k = 0; k < N; k++) if (k != 5 && k != N - 5) rest = fmax(rest, hypot(out[k][0], out[k][1]));
  QT_CHECK(fabs(out[5][0] - N / 2.0) < 1e-9 && fabs(out[5][1]) < 1e-9, "cos(5): X[5] = N/2");
  QT_CHECK(fabs(out[N - 5][0] - N / 2.0) < 1e-9, "cos(5): X[N-5] = N/2");
  QT_CHECK(rest < 1e-9, "cos(5): every other bin is 0");
  fftw_execute(q);
  double rt = 0;
  for (int n = 0; n < N; n++) rt = fmax(rt, fabs(back[n][0] / N - in[n][0]) + fabs(back[n][1] / N));
  QT_CHECK(rt < 1e-12, "forward then backward = N * input");

  /* unit impulse at n=0: X[k] = 1 for all k */
  for (int n = 0; n < N; n++) in[n][0] = in[n][1] = 0;
  in[0][0] = 1;
  fftw_execute(p);
  double flat = 0;
  for (int k = 0; k < N; k++) flat = fmax(flat, hypot(out[k][0] - 1, out[k][1]));
  QT_CHECK(flat < 1e-12, "impulse: flat spectrum of 1");
  /* impulse at n=1: X[k] = exp(-2 pi i k / N) */
  in[0][0] = 0; in[1][0] = 1;
  fftw_execute(p);
  QT_CHECK(fabs(out[N / 4][0]) < 1e-12 && fabs(out[N / 4][1] + 1) < 1e-12, "shifted impulse: X[N/4] = -i");

  /* real-to-complex of a constant 3: X[0] = 3N, rest 0, N/2+1 outputs */
  double *rin = fftw_alloc_real(N);
  for (int n = 0; n < N; n++) rin[n] = 3;
  fftw_plan pr = fftw_plan_dft_r2c_1d(N, rin, out, FFTW_ESTIMATE);
  fftw_execute(pr);
  double r0 = 0;
  for (int k = 1; k <= N / 2; k++) r0 = fmax(r0, hypot(out[k][0], out[k][1]));
  QT_CHECK(fabs(out[0][0] - 3 * N) < 1e-9 && r0 < 1e-9, "r2c of constant 3: X[0] = 3N, others 0");

  /* DCT-II (REDFT10) of a constant c: Y[0] = 2 N c, others 0 */
  double *dout = fftw_alloc_real(N);
  fftw_plan pd = fftw_plan_r2r_1d(N, rin, dout, FFTW_REDFT10, FFTW_ESTIMATE);
  fftw_execute(pd);
  double d0 = 0;
  for (int k = 1; k < N; k++) d0 = fmax(d0, fabs(dout[k]));
  QT_CHECK(fabs(dout[0] - 2.0 * N * 3) < 1e-9 && d0 < 1e-9, "DCT-II of constant: Y[0] = 2N*c");

  /* 2-D 8x8: x[r][c] = cos(2 pi (r + 2c) / 8) -> spikes at (1,2) and (7,6) of 32 */
  fftw_complex *in2 = fftw_alloc_complex(64), *out2 = fftw_alloc_complex(64);
  for (int r = 0; r < 8; r++) for (int c = 0; c < 8; c++) {
    in2[r * 8 + c][0] = cos(2 * PI * (r + 2 * c) / 8); in2[r * 8 + c][1] = 0; }
  fftw_plan p2 = fftw_plan_dft_2d(8, 8, in2, out2, FFTW_FORWARD, FFTW_ESTIMATE);
  fftw_execute(p2);
  QT_CHECK(fabs(out2[1 * 8 + 2][0] - 32) < 1e-9 && fabs(out2[7 * 8 + 6][0] - 32) < 1e-9
           && hypot(out2[0][0], out2[0][1]) < 1e-9, "2-D 8x8: spikes of 32 at (1,2) and (7,6)");

  /* length 1000 (mixed radix) vs. naive DFT, with a measured plan */
  int M = 1000;
  fftw_complex *a = fftw_alloc_complex(M), *A = fftw_alloc_complex(M), *ref = fftw_alloc_complex(M);
  fftw_plan pm = fftw_plan_dft_1d(M, a, A, FFTW_FORWARD, FFTW_MEASURE);
  srand(7);
  for (int n = 0; n < M; n++) { a[n][0] = rand() / (double)RAND_MAX - 0.5; a[n][1] = rand() / (double)RAND_MAX - 0.5; }
  for (int k = 0; k < M; k++) {
    double sr = 0, si = 0;
    for (int n = 0; n < M; n++) {
      double t = -2 * PI * (double)((long)k * n % M) / M;
      sr += a[n][0] * cos(t) - a[n][1] * sin(t);
      si += a[n][0] * sin(t) + a[n][1] * cos(t);
    }
    ref[k][0] = sr; ref[k][1] = si;
  }
  fftw_execute(pm);
  double em = maxerr(A, ref, M);
  printf("  info N=1000 max |FFT - DFT| = %.3g\n", em);
  QT_CHECK(em < 1e-9, "N=1000 (FFTW_MEASURE) matches the naive DFT");

  /* threads: the same transform on 4 threads */
  QT_CHECK(fftw_init_threads() != 0, "fftw_init_threads");
  fftw_plan_with_nthreads(4);
  fftw_complex *At = fftw_alloc_complex(M);
  fftw_plan pt = fftw_plan_dft_1d(M, a, At, FFTW_FORWARD, FFTW_ESTIMATE);
  fftw_execute(pt);
  QT_CHECK(maxerr(At, ref, M) < 1e-9, "4-thread plan matches the naive DFT");
  fftw_destroy_plan(pt); fftw_plan_with_nthreads(1);

  /* single precision */
  int F = 256;
  fftwf_complex *fi = fftwf_alloc_complex(F), *fo = fftwf_alloc_complex(F);
  fftwf_plan pf = fftwf_plan_dft_1d(F, fi, fo, FFTW_FORWARD, FFTW_MEASURE);
  for (int n = 0; n < F; n++) { fi[n][0] = (float)cos(2 * PI * 17 * n / F) + 0.25f; fi[n][1] = 0; }
  fftwf_execute(pf);
  float fr = 0;
  for (int k = 0; k < F; k++) if (k != 0 && k != 17 && k != F - 17) fr = fmaxf(fr, hypotf(fo[k][0], fo[k][1]));
  QT_CHECK(fabsf(fo[17][0] - F / 2.0f) < 1e-3f && fabsf(fo[0][0] - F * 0.25f) < 1e-3f && fr < 1e-3f,
           "float: cos(17) + 0.25 -> X[17] = N/2, X[0] = N/4, rest 0");
  char *wis = fftw_export_wisdom_to_string();
  QT_CHECK(wis && strstr(wis, "fftw-3.3") != NULL, "wisdom exported");
  free(wis);

  fftw_destroy_plan(p); fftw_destroy_plan(q); fftw_destroy_plan(pr); fftw_destroy_plan(pd);
  fftw_destroy_plan(p2); fftw_destroy_plan(pm); fftwf_destroy_plan(pf);
  fftw_cleanup_threads();
  QT_END();
}
