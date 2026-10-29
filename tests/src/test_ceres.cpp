#include "common.h"
#include <ceres/ceres.h>
/* Classic Ceres smoke problem: minimise (10 - x)^2, solution x = 10. */
struct Residual {
  template <typename T> bool operator()(const T *const x, T *residual) const {
    residual[0] = T(10.0) - x[0];
    return true;
  }
};
int main(int argc, char **argv) {
  QT_BEGIN("ceres-solver");
  double x = 0.5;
  ceres::Problem problem;
  problem.AddResidualBlock(
      new ceres::AutoDiffCostFunction<Residual, 1, 1>(new Residual), nullptr, &x);
  ceres::Solver::Options options;
  options.linear_solver_type = ceres::DENSE_QR;
  options.minimizer_progress_to_stdout = false;
  ceres::Solver::Summary summary;
  ceres::Solve(options, &problem, &summary);
  QT_CHECK(summary.termination_type == ceres::CONVERGENCE, "solver converged");
  QT_CHECK(std::abs(x - 10.0) < 1e-6, "minimiser found x == 10");
  printf("  info ceres %s, iterations=%d\n", CERES_VERSION_STRING,
         (int)summary.iterations.size());
  (void)argc; (void)argv;
  QT_END();
}
