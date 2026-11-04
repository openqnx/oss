/* ensmallen (header-only, on Armadillo): optimisers that must reach known
 * minima -- L-BFGS on the Rosenbrock function from (-1.2, 1) to (1, 1);
 * gradient descent on (x-3)^2 + (y+1)^2 to (3, -1); Adam (mini-batch, a
 * separable function) on ensmallen's sphere function to 0; simulated
 * annealing (gradient-free) on the Booth function to (1, 3); and a
 * callback that sees every L-BFGS iteration. */
#include "common.h"
#include <ensmallen.hpp>
#include <cmath>

struct Rosenbrock {
  double Evaluate(const arma::mat &x) { return std::pow(1 - x(0), 2) + 100 * std::pow(x(1) - x(0) * x(0), 2); }
  void Gradient(const arma::mat &x, arma::mat &g) {
    g.set_size(2, 1);
    g(0) = -2 * (1 - x(0)) - 400 * x(0) * (x(1) - x(0) * x(0));
    g(1) = 200 * (x(1) - x(0) * x(0));
  }
};
struct Quad {
  double Evaluate(const arma::mat &x) { return std::pow(x(0) - 3, 2) + std::pow(x(1) + 1, 2); }
  void Gradient(const arma::mat &x, arma::mat &g) { g = arma::vec({2 * (x(0) - 3), 2 * (x(1) + 1)}); }
};
struct CountIter {
  int n = 0;
  template <class O, class F, class M> bool StepTaken(O &, F &, M &) { n++; return false; }
};

int main() {
  QT_BEGIN("ensmallen");
  printf("  info ensmallen %s, armadillo %s\n", ens::version::as_string().c_str(), arma::arma_version::as_string().c_str());

  Rosenbrock rb; arma::mat x0 = arma::vec({-1.2, 1.0});
  ens::L_BFGS lbfgs;
  CountIter cb;
  double f = lbfgs.Optimize(rb, x0, cb);
  printf("  info L-BFGS: f = %.3g at (%.6f, %.6f) after %d steps\n", f, x0(0), x0(1), cb.n);
  QT_CHECK(std::fabs(x0(0) - 1) < 1e-4 && std::fabs(x0(1) - 1) < 1e-4 && f < 1e-8, "L-BFGS: Rosenbrock minimum (1, 1)");
  QT_CHECK(cb.n > 5, "callback saw the L-BFGS steps");

  Quad q; arma::mat x1 = arma::vec({0.0, 0.0});
  ens::GradientDescent gd(0.1, 10000, 1e-12);
  gd.Optimize(q, x1);
  QT_CHECK(std::fabs(x1(0) - 3) < 1e-4 && std::fabs(x1(1) + 1) < 1e-4, "gradient descent: (x-3)^2 + (y+1)^2 minimum (3, -1)");

  ens::test::SphereFunction sph(4);
  arma::mat x2 = sph.GetInitialPoint<arma::mat>();
  ens::Adam adam(0.5, 2, 0.7, 0.999, 1e-8, 50000, 1e-5, false);
  double fs = adam.Optimize(sph, x2);
  QT_CHECK(fs < 1e-3 && arma::abs(x2).max() < 0.05, "Adam: 4-D sphere function minimum 0 at the origin");

  /* stochastic: like ensmallen's own SA tests, up to three tries */
  ens::test::BoothFunction booth;
  arma::mat x3; double fb = 1e9; int tries = 0;
  arma::arma_rng::set_seed(1);
  while (fb > 1e-3 && tries++ < 3) {
    x3 = booth.GetInitialPoint<arma::mat>();
    ens::SA<> sa(ens::ExponentialSchedule(), 1000000, 1000., 1000, 100, 1e-10, 3, 1.5, 0.5, 0.3);
    fb = sa.Optimize(booth, x3);
  }
  printf("  info SA: f = %.3g at (%.4f, %.4f), %d tries\n", fb, x3(0), x3(1), tries);
  QT_CHECK(std::fabs(x3(0) - 1) < 0.05 && std::fabs(x3(1) - 3) < 0.05, "simulated annealing: Booth minimum (1, 3)");
  QT_END();
}
