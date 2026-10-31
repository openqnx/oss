/* OpenBLAS: BLAS level 1/2/3 with known answers (ddot, daxpy, dgemv, dgemm,
 * sgemm), a larger dgemm checked against a naive triple loop (exercises the
 * threaded kernels), and LAPACKE: dgesv solves a known system, dsyev gives
 * known eigenvalues, dgetrf/dgetri inverts a matrix. */
#include "common.h"
#include <math.h>
#include <stdlib.h>
#include <cblas.h>
#include <lapacke.h>
static int close_to(double a, double b, double tol) { return fabs(a - b) <= tol * (1.0 + fabs(b)); }
int main(void) {
  QT_BEGIN("openblas");
  const char *cfg = openblas_get_config();
  printf("  info %s; core %s; threads %d\n", cfg ? cfg : "?", openblas_get_corename() ? openblas_get_corename() : "?",
         openblas_get_num_threads());
  QT_CHECK(openblas_get_num_threads() >= 1, "thread count reported");

  double x[] = { 1, 2, 3, 4 }, y[] = { 5, 6, 7, 8 };
  QT_CHECK(cblas_ddot(4, x, 1, y, 1) == 70.0, "ddot([1..4],[5..8]) = 70");
  cblas_daxpy(4, 2.0, x, 1, y, 1);
  QT_CHECK(y[0] == 7 && y[1] == 10 && y[2] == 13 && y[3] == 16, "daxpy y += 2x");
  QT_CHECK(close_to(cblas_dnrm2(2, (double[]){ 3, 4 }, 1), 5.0, 1e-15), "dnrm2([3,4]) = 5");
  QT_CHECK(cblas_idamax(4, (double[]){ 1, -9, 3, 4 }, 1) == 1, "idamax finds |-9|");

  double A[] = { 1, 2, 3, 4, 5, 6 };   /* 2x3 row-major */
  double v[] = { 1, 0, -1 }, r[2] = { 0 };
  cblas_dgemv(CblasRowMajor, CblasNoTrans, 2, 3, 1.0, A, 3, v, 1, 0.0, r, 1);
  QT_CHECK(r[0] == -2 && r[1] == -2, "dgemv [[1,2,3],[4,5,6]]*[1,0,-1] = [-2,-2]");

  double B[] = { 7, 8, 9, 10, 11, 12 }; /* 3x2 */
  double C[4] = { 0 };
  cblas_dgemm(CblasRowMajor, CblasNoTrans, CblasNoTrans, 2, 2, 3, 1.0, A, 3, B, 2, 0.0, C, 2);
  printf("  info dgemm = [%g %g; %g %g]\n", C[0], C[1], C[2], C[3]);
  QT_CHECK(C[0] == 58 && C[1] == 64 && C[2] == 139 && C[3] == 154, "dgemm 2x3 * 3x2 = [58 64; 139 154]");
  float fa[] = { 1, 2, 3, 4 }, fb[] = { 5, 6, 7, 8 }, fc[4] = { 0 };
  cblas_sgemm(CblasColMajor, CblasNoTrans, CblasNoTrans, 2, 2, 2, 1.0f, fa, 2, fb, 2, 0.0f, fc, 2);
  QT_CHECK(fc[0] == 23 && fc[1] == 34 && fc[2] == 31 && fc[3] == 46, "sgemm column-major 2x2 product");

  int n = 160;
  double *M = malloc(sizeof(double) * n * n), *N = malloc(sizeof(double) * n * n), *P = malloc(sizeof(double) * n * n);
  for (int i = 0; i < n * n; i++) { M[i] = (double)((i * 7) % 13) - 6; N[i] = (double)((i * 5) % 11) - 5; }
  cblas_dgemm(CblasRowMajor, CblasNoTrans, CblasNoTrans, n, n, n, 1.0, M, n, N, n, 0.0, P, n);
  int bad = 0;
  for (int i = 0; i < n; i += 7) for (int j = 0; j < n; j += 5) {
    double s = 0; for (int k = 0; k < n; k++) s += M[i * n + k] * N[k * n + j];
    if (s != P[i * n + j]) bad++;   /* small integers: exact in double */
  }
  QT_CHECK(bad == 0, "dgemm 160x160 matches the naive product exactly");
  free(M); free(N); free(P);

  /* 2x+y+z=4, x+3y+2z=5, x=6 */
  double S[] = { 2, 1, 1, 1, 3, 2, 1, 0, 0 };
  double rhs[] = { 4, 5, 6 };   /* row-major; solution x=6, y=15, z=-23 */
  lapack_int ipiv[3];
  lapack_int info = LAPACKE_dgesv(LAPACK_ROW_MAJOR, 3, 1, S, 3, ipiv, rhs, 1);
  printf("  info dgesv info=%d x=[%g %g %g]\n", (int)info, rhs[0], rhs[1], rhs[2]);
  QT_CHECK(info == 0 && close_to(rhs[0], 6, 1e-12) && close_to(rhs[1], 15, 1e-12) && close_to(rhs[2], -23, 1e-12),
           "LAPACKE_dgesv solves a 3x3 system");

  double E[] = { 2, -1, 0, -1, 2, -1, 0, -1, 2 }, w[3];
  info = LAPACKE_dsyev(LAPACK_ROW_MAJOR, 'N', 'U', 3, E, 3, w);
  printf("  info eigenvalues %.12f %.12f %.12f\n", w[0], w[1], w[2]);
  QT_CHECK(info == 0 && close_to(w[0], 2 - sqrt(2.0), 1e-12) && close_to(w[1], 2, 1e-12) && close_to(w[2], 2 + sqrt(2.0), 1e-12),
           "LAPACKE_dsyev: tridiag(-1,2,-1) eigenvalues 2-sqrt2, 2, 2+sqrt2");

  double I2[] = { 4, 7, 2, 6 };
  lapack_int p2[2];
  info = LAPACKE_dgetrf(LAPACK_ROW_MAJOR, 2, 2, I2, 2, p2);
  if (info == 0) info = LAPACKE_dgetri(LAPACK_ROW_MAJOR, 2, I2, 2, p2);
  QT_CHECK(info == 0 && close_to(I2[0], 0.6, 1e-14) && close_to(I2[1], -0.7, 1e-14) && close_to(I2[2], -0.2, 1e-14) &&
           close_to(I2[3], 0.4, 1e-14), "dgetrf+dgetri inverse of [[4,7],[2,6]]");
  double sing[] = { 1, 2, 2, 4 }, sb[] = { 1, 1 };
  lapack_int p3[2];
  QT_CHECK(LAPACKE_dgesv(LAPACK_ROW_MAJOR, 2, 1, sing, 2, p3, sb, 1) > 0, "dgesv reports a singular matrix");
  QT_END();
}
