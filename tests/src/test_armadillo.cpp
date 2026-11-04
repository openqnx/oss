/* armadillo: linear algebra with known answers, through libarmadillo and
 * OpenBLAS (BLAS + LAPACK) -- determinant and inverse of a 2x2, a linear
 * solve, symmetric eigenvalues (1 and 3 of [[2,1],[1,2]]), complex
 * eigenvalues +-i of a rotation, singular values, Cholesky R with
 * R'R = A and the exact factor, QR (Q orthogonal, QR = A), LU with
 * pivoting, a least-squares line fit recovering y = 2x + 1, and a
 * 200x200 product (dgemm) and SPD solve checked by residual. */
#include "common.h"
#include <armadillo>
#include <cmath>
#include <complex>

int main() {
  QT_BEGIN("armadillo");
  printf("  info armadillo %s\n", arma::arma_version::as_string().c_str());
  using namespace arma;
  mat A = {{4, 1}, {2, 3}};
  QT_CHECK(std::fabs(det(A) - 10) < 1e-12, "det [[4,1],[2,3]] = 10");
  mat Ai = inv(A), Aw = mat({{0.3, -0.1}, {-0.2, 0.4}});
  QT_CHECK(approx_equal(Ai, Aw, "absdiff", 1e-12), "inverse = [[0.3,-0.1],[-0.2,0.4]]");
  vec x = solve(A, vec({1, 2}));
  QT_CHECK(approx_equal(x, vec({0.1, 0.6}), "absdiff", 1e-12), "solve A x = [1 2]: x = [0.1 0.6]");

  vec ev = eig_sym(mat({{2, 1}, {1, 2}}));
  QT_CHECK(approx_equal(ev, vec({1, 3}), "absdiff", 1e-12), "eig_sym [[2,1],[1,2]] = {1, 3}");
  cx_vec cev = eig_gen(mat({{0, -1}, {1, 0}}));
  QT_CHECK(std::fabs(std::abs(cev(0)) - 1) < 1e-12 && std::fabs(cev(0).real()) < 1e-12
           && std::fabs(cev(0).imag() + cev(1).imag()) < 1e-12 && std::fabs(std::fabs(cev(0).imag()) - 1) < 1e-12,
           "eig_gen of a 90-degree rotation = +-i");

  vec sv = svd(mat({{3, 0}, {0, -4}}));
  QT_CHECK(approx_equal(sv, vec({4, 3}), "absdiff", 1e-12), "singular values of diag(3,-4) = {4, 3}");
  mat U, V; vec s;
  mat B = {{1, 2}, {3, 4}, {5, 6}};
  QT_CHECK(svd(U, s, V, B) && approx_equal(U.cols(0, 1) * diagmat(s) * V.t(), B, "absdiff", 1e-10), "full SVD reconstructs B");

  mat S = {{4, 2}, {2, 3}};
  mat R = chol(S);
  QT_CHECK(approx_equal(R, mat({{2, 1}, {0, std::sqrt(2.0)}}), "absdiff", 1e-12), "chol [[4,2],[2,3]] = [[2,1],[0,sqrt2]]");
  QT_CHECK(approx_equal(R.t() * R, S, "absdiff", 1e-12), "R'R = A");

  mat Q, Rq;
  qr(Q, Rq, B);
  QT_CHECK(approx_equal(Q * Rq, B, "absdiff", 1e-12) && approx_equal(Q.t() * Q, eye(3, 3), "absdiff", 1e-12), "QR: QR = B, Q'Q = I");
  mat L, Uu, P;
  lu(L, Uu, P, A);
  QT_CHECK(approx_equal(P.t() * L * Uu, A, "absdiff", 1e-12), "LU with pivoting: P'LU = A");

  /* least squares: points on y = 2x + 1 */
  vec xs = linspace(0, 9, 10);
  mat X = join_rows(ones(10), xs);
  vec coef = solve(X, 2 * xs + 1);
  QT_CHECK(approx_equal(coef, vec({1, 2}), "absdiff", 1e-10), "least-squares fit recovers intercept 1, slope 2");

  /* larger: dgemm and an SPD solve */
  arma_rng::set_seed(42);
  mat M = randu(200, 200), N2 = randu(200, 200);
  mat P2 = M * N2;
  double e = 0;
  for (int i = 0; i < 200; i += 37) for (int j = 0; j < 200; j += 41) {
    double sum = 0; for (int k = 0; k < 200; k++) sum += M(i, k) * N2(k, j);
    e = std::fmax(e, std::fabs(sum - P2(i, j)));
  }
  QT_CHECK(e < 1e-10, "200x200 product (BLAS dgemm) matches naive sums");
  mat SPD = M.t() * M + 200 * eye(200, 200);
  vec b = randu(200);
  vec y = solve(SPD, b, solve_opts::likely_sympd);
  double res = norm(SPD * y - b) / norm(b);
  printf("  info SPD 200x200 relative residual %.2g\n", res);
  QT_CHECK(res < 1e-12, "200x200 SPD solve residual < 1e-12");
  QT_CHECK(std::fabs(trace(eye(5, 5) * 3) - 15) < 1e-15 && accu(ones(3, 4)) == 12, "trace and accu");
  QT_END();
}
