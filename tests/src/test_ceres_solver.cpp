/* Ceres Solver: three problems with known answers -- exponential curve
 * fitting on noise-free data generated from y = exp(0.3x + 0.1), the
 * Rosenbrock function via the gradient solver, and a robust (Cauchy) line
 * fit that must ignore gross outliers -- plus a numeric-vs-automatic
 * derivative cross-check. */
#include "common.h"
#include <cmath>
#include <vector>
#include <ceres/ceres.h>

struct ExpResidual {
  ExpResidual(double x, double y) : x_(x), y_(y) {}
  template <typename T> bool operator()(const T *const m, const T *const c, T *r) const {
    r[0] = T(y_) - exp(m[0] * T(x_) + c[0]);
    return true;
  }
  double x_, y_;
};
struct Rosenbrock {
  template <typename T> bool operator()(const T *p, T *cost) const {
    T x = p[0], y = p[1];
    cost[0] = (1.0 - x) * (1.0 - x) + 100.0 * (y - x * x) * (y - x * x);
    return true;
  }
};
struct LineResidual {
  LineResidual(double x, double y) : x_(x), y_(y) {}
  template <typename T> bool operator()(const T *ab, T *r) const { r[0] = T(y_) - (ab[0] * T(x_) + ab[1]); return true; }
  double x_, y_;
};
struct SinResidual {
  bool operator()(const double *x, double *r) const { r[0] = std::sin(x[0]) * x[1]; return true; }
};

int main() {
  QT_BEGIN("ceres-solver");
  printf("  info ceres %s\n", CERES_VERSION_STRING);

  double m = 0.0, c = 0.0;
  ceres::Problem p1;
  for (int i = 0; i < 40; ++i) {
    double x = i * 0.125, y = std::exp(0.3 * x + 0.1);
    p1.AddResidualBlock(new ceres::AutoDiffCostFunction<ExpResidual, 1, 1, 1>(new ExpResidual(x, y)), nullptr, &m, &c);
  }
  ceres::Solver::Options o;
  o.linear_solver_type = ceres::DENSE_QR;
  o.max_num_iterations = 100;
  o.function_tolerance = 1e-14; o.parameter_tolerance = 1e-14; o.gradient_tolerance = 1e-16;
  ceres::Solver::Summary s1;
  ceres::Solve(o, &p1, &s1);
  printf("  info m=%.9f c=%.9f after %d iterations\n", m, c, (int)s1.iterations.size());
  QT_CHECK(s1.IsSolutionUsable(), "curve fit: solution usable");
  QT_CHECK(std::fabs(m - 0.3) < 1e-6 && std::fabs(c - 0.1) < 1e-6, "curve fit recovers m = 0.3, c = 0.1");
  QT_CHECK(s1.final_cost < 1e-12, "curve fit residual ~ 0");

  double pr[2] = {-1.2, 1.0};
  ceres::GradientProblem gp(new ceres::AutoDiffFirstOrderFunction<Rosenbrock, 2>(new Rosenbrock));
  ceres::GradientProblemSolver::Options go;
  go.max_num_iterations = 1000;
  ceres::GradientProblemSolver::Summary gs;
  ceres::Solve(go, gp, pr, &gs);
  printf("  info Rosenbrock -> (%.6f, %.6f)\n", pr[0], pr[1]);
  QT_CHECK(std::fabs(pr[0] - 1.0) < 1e-4 && std::fabs(pr[1] - 1.0) < 1e-4, "Rosenbrock minimum at (1, 1) via L-BFGS");

  double ab[2] = {0, 0};
  ceres::Problem p3;
  for (int i = 0; i < 30; ++i) {
    double x = i, y = 2.0 * x - 5.0;
    if (i % 10 == 3) y += 400.0;   // three gross outliers
    p3.AddResidualBlock(new ceres::AutoDiffCostFunction<LineResidual, 1, 2>(new LineResidual(x, y)),
                        new ceres::CauchyLoss(1.0), ab);
  }
  ceres::Solver::Summary s3;
  ceres::Solve(o, &p3, &s3);
  printf("  info robust line a=%.5f b=%.5f\n", ab[0], ab[1]);
  QT_CHECK(std::fabs(ab[0] - 2.0) < 1e-2 && std::fabs(ab[1] + 5.0) < 0.2, "Cauchy loss ignores outliers: y = 2x - 5");

  ceres::NumericDiffCostFunction<SinResidual, ceres::CENTRAL, 1, 2> nd(new SinResidual);
  double xv[2] = {0.5, 3.0}; double *pp[1] = {xv};
  double r, jac[2]; double *jj[1] = {jac};
  nd.Evaluate(pp, &r, jj);
  QT_CHECK(std::fabs(jac[0] - std::cos(0.5) * 3.0) < 1e-6 && std::fabs(jac[1] - std::sin(0.5)) < 1e-6,
           "central-difference Jacobian matches the analytic one");
  QT_END();
}
