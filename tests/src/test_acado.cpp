// acado: optimal control with known optima, and MPC code generation.
//  - min integral of u^2 over [0, 1], x' = u, x(0) = 0, x(1) = 1: the
//    optimal control is u = 1 throughout, objective 1 (exact for ACADO's
//    piecewise-constant controls).
//  - The same with a bound u <= 0.8 and x(1) = 0.6: u = 0.6, objective 0.36.
//  - Code generation (OCPexport): an MPC solver for a double integrator is
//    exported to a directory; ACADO finds its templates in the installed
//    tree (share/acado/templates) and writes
//    acado_solver.c, acado_common.h (with ACADO_N 20, ACADO_NX 2) etc.
#include "common.h"
#include <cmath>
#include <cstdlib>
#include <fstream>
#include <sstream>
#include <string>
#include <sys/stat.h>
#include <acado_optimal_control.hpp>
#include <acado_code_generation.hpp>
USING_NAMESPACE_ACADO

static bool solve(double xend, double umax, double &obj, double &umin_, double &umax_) {
  // ACADO numbers symbolic variables globally: every problem in a process
  // starts by resetting the counters, or its states index past the first one's.
  clearAllStaticCounters();
  DifferentialState x; Control u;
  DifferentialEquation f(0.0, 1.0);
  f << dot(x) == u;
  OCP ocp(0.0, 1.0, 10);
  ocp.minimizeLagrangeTerm(u * u);
  ocp.subjectTo(f);
  ocp.subjectTo(AT_START, x == 0.0);
  ocp.subjectTo(AT_END, x == xend);
  if (umax > 0) ocp.subjectTo(u <= umax);
  OptimizationAlgorithm alg(ocp);
  alg.set(PRINTLEVEL, NONE);
  alg.set(PRINT_COPYRIGHT, BT_FALSE);
  alg.set(KKT_TOLERANCE, 1e-10);
  returnValue r = alg.solve();
  VariablesGrid ug; alg.getControls(ug);
  obj = alg.getObjectiveValue();
  umin_ = 1e9; umax_ = -1e9;
  for (unsigned i = 0; i < ug.getNumPoints(); i++) {
    double v = ug(i, 0); if (v < umin_) umin_ = v; if (v > umax_) umax_ = v;
  }
  return r == SUCCESSFUL_RETURN;
}

static bool has(const std::string &p, const char *needle) {
  std::ifstream f(p); std::stringstream s; s << f.rdbuf();
  return s.str().find(needle) != std::string::npos;
}

int main() {
  QT_BEGIN("acado");
  char m[200];
  double obj, lo, hi;
  bool ok = solve(1.0, -1, obj, lo, hi);
  snprintf(m, sizeof m, "OCP x'=u, x(1)=1: objective %.8f, u in [%.6f, %.6f] (1, u = 1)", obj, lo, hi);
  QT_CHECK(ok && std::fabs(obj - 1) < 1e-6 && std::fabs(lo - 1) < 1e-5 && std::fabs(hi - 1) < 1e-5, m);
  ok = solve(0.6, 0.8, obj, lo, hi);
  snprintf(m, sizeof m, "OCP x(1)=0.6, u<=0.8: objective %.8f, u in [%.6f, %.6f] (0.36, u = 0.6)", obj, lo, hi);
  QT_CHECK(ok && std::fabs(obj - 0.36) < 1e-6 && std::fabs(lo - 0.6) < 1e-5 && std::fabs(hi - 0.6) < 1e-5, m);

  const char *tmp = getenv("TMPDIR");
  std::string dir = std::string(tmp ? tmp : "/tmp") + "/acado_export_test";
  {
    clearAllStaticCounters();
    DifferentialState p, v; Control a;
    DifferentialEquation f;
    f << dot(p) == v;
    f << dot(v) == a;
    Function h; h << p << v << a;
    Function hN; hN << p << v;
    DMatrix W = eye<double>(3), WN = eye<double>(2);
    OCP ocp(0.0, 2.0, 20);
    ocp.subjectTo(f);
    ocp.minimizeLSQ(W, h);
    ocp.minimizeLSQEndTerm(WN, hN);
    ocp.subjectTo(-1.0 <= a <= 1.0);
    OCPexport mpc(ocp);
    mpc.set(HESSIAN_APPROXIMATION, GAUSS_NEWTON);
    mpc.set(DISCRETIZATION_TYPE, MULTIPLE_SHOOTING);
    mpc.set(SPARSE_QP_SOLUTION, FULL_CONDENSING);
    mpc.set(INTEGRATOR_TYPE, INT_RK4);
    mpc.set(NUM_INTEGRATOR_STEPS, 40);
    mpc.set(QP_SOLVER, QP_QPOASES);
    mpc.set(GENERATE_TEST_FILE, NO);
    mpc.set(GENERATE_MAKE_FILE, NO);
    mpc.set(PRINTLEVEL, NONE);
    returnValue r = mpc.exportCode(dir.c_str());
    QT_CHECK(r == SUCCESSFUL_RETURN, "OCPexport::exportCode of a double-integrator MPC");
  }
  struct stat st;
  QT_CHECK(stat((dir + "/acado_solver.c").c_str(), &st) == 0 && st.st_size > 1000, "acado_solver.c written");
  QT_CHECK(has(dir + "/acado_common.h", "#define ACADO_N 20") && has(dir + "/acado_common.h", "#define ACADO_NX 2"),
           "acado_common.h defines ACADO_N 20 and ACADO_NX 2");
  QT_CHECK(stat((dir + "/acado_qpoases_interface.cpp").c_str(), &st) == 0, "the qpOASES interface was exported from the installed templates");
  std::string rm = "rm -rf " + dir; if (system(rm.c_str())) {}
  QT_END();
}
