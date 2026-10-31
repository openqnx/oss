/* CLAPACK (f2c'd LAPACK + BLAS): known-answer linear algebra -- DGEMM matrix
 * product, DDOT, DGESV solving a 3x3 system, DGETRF determinant, DPOTRF
 * Cholesky factor and DSYEV eigenvalues of a symmetric matrix. Matrices are
 * column-major, as Fortran expects. */
#include "common.h"
#include <math.h>
#include <f2c.h>
#include <clapack.h>
#undef abs
#undef min
#undef max

/* libf2c's Fortran main() calls MAIN__; it is never used here, but libf2c.so
   leaves the symbol for the program to provide. */
int MAIN__(void) { return 0; }

/* LAPACK's error handler; ours replaces the library's printing one. */
static char xerbla_name[8]; static long xerbla_info;
int xerbla_(char *srname, integer *info) {
  memcpy(xerbla_name, srname, 6); xerbla_name[6] = 0; xerbla_info = (long)*info;
  return 0;
}

static int near(double a, double b) { return fabs(a - b) < 1e-10; }

int main(void) {
  QT_BEGIN("clapack");
  /* C = A*B, A 2x3, B 3x2 (column-major) */
  double A[6] = {1, 4, 2, 5, 3, 6};   /* [[1,2,3],[4,5,6]] */
  double B[6] = {7, 9, 11, 8, 10, 12}; /* [[7,8],[9,10],[11,12]] */
  double C[4] = {0};
  integer m = 2, n = 2, k = 3, lda = 2, ldb = 3, ldc = 2;
  double one = 1.0, zero = 0.0;
  char N = 'N';
  dgemm_(&N, &N, &m, &n, &k, &one, A, &lda, B, &ldb, &zero, C, &ldc);
  QT_CHECK(C[0] == 58 && C[1] == 139 && C[2] == 64 && C[3] == 154, "DGEMM: [[58,64],[139,154]]");

  double x[3] = {1, 2, 3}, y[3] = {4, -5, 6};
  integer three = 3, inc = 1;
  QT_CHECK(ddot_(&three, x, &inc, y, &inc) == 12.0, "DDOT = 12");

  /* 2x + y - z = 8; -3x - y + 2z = -11; -2x + y + 2z = -3  =>  (2, 3, -1) */
  double M[9] = {2, -3, -2, 1, -1, 1, -1, 2, 2};
  double rhs[3] = {8, -11, -3};
  integer ipiv[3], info = -1, nrhs = 1, ld3 = 3;
  dgesv_(&three, &nrhs, M, &ld3, ipiv, rhs, &ld3, &info);
  printf("  info DGESV x = (%g, %g, %g), info %ld\n", rhs[0], rhs[1], rhs[2], (long)info);
  QT_CHECK(info == 0 && near(rhs[0], 2) && near(rhs[1], 3) && near(rhs[2], -1), "DGESV solves to (2, 3, -1)");

  double D[9] = {6, 4, 2, 1, -2, 8, 1, 5, 7};   /* [[6,1,1],[4,-2,5],[2,8,7]], det = -306 */
  dgetrf_(&three, &three, D, &ld3, ipiv, &info);
  double det = 1;
  for (int i = 0; i < 3; ++i) { det *= D[i * 4]; if (ipiv[i] != i + 1) det = -det; }
  printf("  info det = %g\n", det);
  QT_CHECK(info == 0 && near(det, -306), "DGETRF determinant = -306");

  double P[9] = {4, 12, -16, 12, 37, -43, -16, -43, 98};   /* L = [[2,0,0],[6,1,0],[-8,5,3]] */
  char L = 'L';
  dpotrf_(&L, &three, P, &ld3, &info);
  QT_CHECK(info == 0 && near(P[0], 2) && near(P[1], 6) && near(P[2], -8) && near(P[4], 1) && near(P[5], 5) && near(P[8], 3),
           "DPOTRF Cholesky factor");

  double S[9] = {2, -1, 0, -1, 2, -1, 0, -1, 2};   /* eigenvalues 2-sqrt2, 2, 2+sqrt2 */
  double w[3], work[64];
  integer lwork = 64;
  char V = 'V', U = 'U';
  dsyev_(&V, &U, &three, S, &ld3, w, work, &lwork, &info);
  printf("  info eigenvalues %.12f %.12f %.12f\n", w[0], w[1], w[2]);
  QT_CHECK(info == 0 && near(w[0], 2 - sqrt(2.0)) && near(w[1], 2) && near(w[2], 2 + sqrt(2.0)), "DSYEV eigenvalues");
  /* eigenvector of 2: (1, 0, -1)/sqrt2 up to sign */
  QT_CHECK(near(fabs(S[3]), 1 / sqrt(2.0)) && near(S[4], 0) && near(S[3], -S[5]), "DSYEV eigenvector for lambda = 2");

  integer bad = -1;
  dgesv_(&bad, &nrhs, M, &ld3, ipiv, rhs, &ld3, &info);
  QT_CHECK(info == -1, "invalid N reported through INFO (XERBLA path)");
  printf("  info XERBLA called for \"%s\", parameter %ld\n", xerbla_name, xerbla_info);
  QT_END();
}
