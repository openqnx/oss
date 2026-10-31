/* SuiteSparse: the 5x5 unsymmetric system of UMFPACK's own demo (solution
 * 1..5) solved with UMFPACK, KLU and CXSparse; an SPD tridiagonal system
 * solved with CHOLMOD (supernodal/simplicial Cholesky, AMD ordering); and
 * AMD ordering an arrow matrix, which must put the dense node last. */
#include "common.h"
#include <math.h>
#include <stdlib.h>
#include <suitesparse/SuiteSparse_config.h>
#include <suitesparse/umfpack.h>
#include <suitesparse/klu.h>
#include <suitesparse/amd.h>
#include <suitesparse/cholmod.h>
#include <suitesparse/cs.h>
static int32_t Ap[] = { 0, 2, 5, 9, 10, 12 };
static int32_t Ai[] = { 0, 1, 0, 2, 4, 1, 2, 3, 4, 2, 1, 4 };
static double Ax[] = { 2., 3., 3., -1., 4., 4., -3., 1., 2., 2., 6., 1. };
static double b[] = { 8., 45., -3., 3., 19. };
static int is_1_to_5(const double *x) {
  for (int i = 0; i < 5; i++) if (fabs(x[i] - (i + 1)) > 1e-10) return 0;
  return 1;
}
int main(void) {
  QT_BEGIN("suitesparse");
  int v[3];
  SuiteSparse_version(v);
  printf("  info SuiteSparse %d.%d.%d\n", v[0], v[1], v[2]);
  QT_CHECK(v[0] == SUITESPARSE_MAIN_VERSION && v[1] == SUITESPARSE_SUB_VERSION, "library version matches header");

  double x[5] = { 0 };
  void *sym = NULL, *num = NULL;
  int st = umfpack_di_symbolic(5, 5, Ap, Ai, Ax, &sym, NULL, NULL);
  if (st == UMFPACK_OK) st = umfpack_di_numeric(Ap, Ai, Ax, sym, &num, NULL, NULL);
  if (st == UMFPACK_OK) st = umfpack_di_solve(UMFPACK_A, Ap, Ai, Ax, x, b, num, NULL, NULL);
  printf("  info umfpack status %d x = %g %g %g %g %g\n", st, x[0], x[1], x[2], x[3], x[4]);
  QT_CHECK(st == UMFPACK_OK && is_1_to_5(x), "UMFPACK solves Ax=b (x = 1..5)");
  umfpack_di_free_symbolic(&sym); umfpack_di_free_numeric(&num);

  klu_common kc; klu_defaults(&kc);
  double kb[5]; memcpy(kb, b, sizeof b);
  klu_symbolic *ks = klu_analyze(5, Ap, Ai, &kc);
  klu_numeric *kn = ks ? klu_factor(Ap, Ai, Ax, ks, &kc) : NULL;
  int kok = kn && klu_solve(ks, kn, 5, 1, kb, &kc);
  QT_CHECK(kok && is_1_to_5(kb), "KLU solves the same system");
  if (kn) { double rc = 0; klu_rcond(ks, kn, &kc); rc = kc.rcond; printf("  info KLU rcond %.3g, %d blocks\n", rc, ks->nblocks); }
  klu_free_numeric(&kn, &kc); klu_free_symbolic(&ks, &kc);

  cs_di A = { 12, 5, 5, Ap, Ai, Ax, -1 };
  double cb[5]; memcpy(cb, b, sizeof b);
  QT_CHECK(cs_di_lusol(1, &A, cb, 1e-12) && is_1_to_5(cb), "CXSparse cs_lusol solves the same system");

  /* arrow matrix: node 0 connected to everything; AMD should order it last */
  int32_t arP[] = { 0, 6, 8, 10, 12, 14, 16 };
  int32_t arI[] = { 0, 1, 2, 3, 4, 5, 0, 1, 0, 2, 0, 3, 0, 4, 0, 5 };
  int32_t perm[6];
  double ctl[AMD_CONTROL], inf[AMD_INFO];
  amd_defaults(ctl);
  st = amd_order(6, arP, arI, perm, ctl, inf);
  printf("  info AMD perm %d %d %d %d %d %d, nnz(L) %.0f\n", perm[0], perm[1], perm[2], perm[3], perm[4], perm[5], inf[AMD_LNZ]);
  QT_CHECK(st == AMD_OK && perm[5] == 0, "AMD orders the dense arrow node last");
  QT_CHECK(inf[AMD_LNZ] == 5, "AMD ordering gives fill-free factor (nnz(L)=5)");

  /* CHOLMOD: tridiag(-1, 2, -1) of size 50, b = A * (1..50) */
  cholmod_common cc;
  cholmod_start(&cc);
  int n = 50;
  cholmod_triplet *T = cholmod_allocate_triplet(n, n, 2 * n - 1, -1 /* lower */, CHOLMOD_REAL, &cc);
  int32_t *ti = T->i, *tj = T->j; double *tx = T->x;
  for (int i = 0; i < n; i++) {
    ti[T->nnz] = i; tj[T->nnz] = i; tx[T->nnz++] = 2.0;
    if (i + 1 < n) { ti[T->nnz] = i + 1; tj[T->nnz] = i; tx[T->nnz++] = -1.0; }
  }
  cholmod_sparse *S = cholmod_triplet_to_sparse(T, 0, &cc);
  cholmod_dense *B = cholmod_allocate_dense(n, 1, n, CHOLMOD_REAL, &cc);
  double *bx = B->x;
  for (int i = 0; i < n; i++) bx[i] = 2.0 * (i + 1) - (i > 0 ? i : 0) - (i + 1 < n ? i + 2 : 0);
  cholmod_factor *L = cholmod_analyze(S, &cc);
  int fok = L && cholmod_factorize(S, L, &cc);
  cholmod_dense *X = fok ? cholmod_solve(CHOLMOD_A, L, B, &cc) : NULL;
  int xok = X != NULL;
  if (X) { double *xx = X->x; for (int i = 0; i < n; i++) if (fabs(xx[i] - (i + 1)) > 1e-9) xok = 0; }
  printf("  info CHOLMOD status %d, ordering %d, is_super %d\n", cc.status, L ? L->ordering : -1, L ? L->is_super : -1);
  QT_CHECK(fok && cc.status == CHOLMOD_OK, "CHOLMOD analyze + factorize (SPD)");
  QT_CHECK(xok, "CHOLMOD solve gives x = 1..50");
  cholmod_free_dense(&X, &cc); cholmod_free_dense(&B, &cc); cholmod_free_factor(&L, &cc);
  cholmod_free_sparse(&S, &cc); cholmod_free_triplet(&T, &cc);
  cholmod_finish(&cc);
  QT_END();
}
