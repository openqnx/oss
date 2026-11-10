// casadi: symbolic AD, NLP solving and integration with known answers.
//  - AD: d/dx [x^2 sin x] at x = 1 is 2 sin 1 + cos 1; the Hessian of
//    x^2 y + y^3 at (1, 2) is [[4, 2], [2, 12]].
//  - NLP (SQP method with the qrqp QP solver, both loaded as plugins):
//    Rosenbrock from (-1.2, 1) to its minimum (1, 1), f = 0; and
//    min x^2 + y^2 s.t. x + y >= 1 -> (0.5, 0.5), f = 0.5, multiplier -1.
//  - Integrator (SUNDIALS CVODES plugin): x' = -x, x(0) = 1 -> x(1) = 1/e.
#include "common.h"
#include <cmath>
#include <casadi/casadi.hpp>
using namespace casadi;

int main() {
  QT_BEGIN("casadi");
  char m[200];
  try {
    SX x = SX::sym("x"), y = SX::sym("y");
    Function g("g", {x}, {jacobian(x * x * sin(x), x)});
    double d = double(g(std::vector<DM>{DM(1.0)})[0]);
    snprintf(m, sizeof m, "AD: d/dx x^2 sin x at 1 = %.12f (2 sin 1 + cos 1 = %.12f)", d, 2 * std::sin(1.0) + std::cos(1.0));
    QT_CHECK(std::fabs(d - (2 * std::sin(1.0) + std::cos(1.0))) < 1e-12, m);
    SX z = vertcat(x, y);
    Function h("h", {z}, {hessian(x * x * y + y * y * y, z)});
    DM H = h(std::vector<DM>{DM(std::vector<double>{1, 2})})[0];
    std::vector<double> hv = H.get_elements();
    QT_CHECK(hv.size() == 4 && hv[0] == 4 && hv[1] == 2 && hv[2] == 2 && hv[3] == 12, "AD: Hessian of x^2 y + y^3 at (1,2) is [[4,2],[2,12]]");

    Dict quiet = {{"print_header", false}, {"print_iteration", false}, {"print_status", false}, {"print_time", false},
                  {"qpsol", "qrqp"}, {"qpsol_options", Dict{{"print_iter", false}, {"print_header", false}, {"print_info", false}}},
                  {"max_iter", 200}};
    SXDict ros = {{"x", z}, {"f", pow(1 - x, 2) + 100 * pow(y - x * x, 2)}};
    Function s1 = nlpsol("rosenbrock", "sqpmethod", ros, quiet);
    DMDict r1 = s1(DMDict{{"x0", DM(std::vector<double>{-1.2, 1.0})}});
    std::vector<double> x1 = r1.at("x").get_elements();
    double f1 = double(r1.at("f"));
    snprintf(m, sizeof m, "NLP sqpmethod: Rosenbrock minimum at (%.6f, %.6f), f = %.2e (1, 1, 0)", x1[0], x1[1], f1);
    QT_CHECK(std::fabs(x1[0] - 1) < 1e-5 && std::fabs(x1[1] - 1) < 1e-5 && f1 < 1e-10 && s1.stats().at("success").as_bool(), m);

    SXDict con = {{"x", z}, {"f", x * x + y * y}, {"g", x + y}};
    Function s2 = nlpsol("constrained", "sqpmethod", con, quiet);
    DMDict r2 = s2(DMDict{{"x0", DM(std::vector<double>{3.0, -1.0})}, {"lbg", 1.0}, {"ubg", inf}});
    std::vector<double> x2 = r2.at("x").get_elements();
    double f2 = double(r2.at("f")), lam = double(r2.at("lam_g"));
    snprintf(m, sizeof m, "NLP: min x^2+y^2, x+y>=1 -> (%.6f, %.6f), f = %.6f, lambda = %.6f (0.5, 0.5, 0.5, -1)", x2[0], x2[1], f2, lam);
    QT_CHECK(std::fabs(x2[0] - 0.5) < 1e-6 && std::fabs(x2[1] - 0.5) < 1e-6 && std::fabs(f2 - 0.5) < 1e-6 && std::fabs(lam + 1) < 1e-6, m);

    Function I = integrator("decay", "cvodes", SXDict{{"x", x}, {"ode", -x}}, 0.0, 1.0, Dict{{"abstol", 1e-12}, {"reltol", 1e-12}});
    double xf = double(I(DMDict{{"x0", 1.0}}).at("xf"));
    snprintf(m, sizeof m, "CVODES: x' = -x, x(0) = 1 -> x(1) = %.10f (1/e = %.10f)", xf, std::exp(-1.0));
    QT_CHECK(std::fabs(xf - std::exp(-1.0)) < 1e-8, m);
  } catch (std::exception &e) {
    snprintf(m, sizeof m, "exception: %.150s", e.what());
    QT_CHECK(false, m);
  }
  QT_END();
}
