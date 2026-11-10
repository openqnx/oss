// ruckig: time-optimal, jerk-limited rest-to-rest move of 1 m with
// v, a, j limits of 1. Neither the velocity nor the acceleration limit is
// reached: four jerk phases of t = cbrt(1/2), so the move takes
// 4 * 2^(-1/3) = 3.174802 s. The trajectory ends at 1 m at rest, passes 0.5 m
// at half time (symmetry), and never exceeds the limits. Then a 3-DoF move is
// time-synchronised: every axis finishes at the same (longest) duration.
#include "common.h"
#include <cmath>
#include <ruckig/ruckig.hpp>
using namespace ruckig;

int main() {
  QT_BEGIN("ruckig");
  char m[160];
  {
    InputParameter<1> in;
    in.current_position = {0.0}; in.current_velocity = {0.0}; in.current_acceleration = {0.0};
    in.target_position = {1.0}; in.target_velocity = {0.0}; in.target_acceleration = {0.0};
    in.max_velocity = {1.0}; in.max_acceleration = {1.0}; in.max_jerk = {1.0};
    Ruckig<1> otg; Trajectory<1> traj;
    Result r = otg.calculate(in, traj);
    QT_CHECK(r == Result::Working, "calculate() succeeds");
    double T = traj.get_duration(), want = 4.0 / std::cbrt(2.0);
    snprintf(m, sizeof m, "duration %.6f s, expected 4/cbrt(2) = %.6f s", T, want);
    QT_CHECK(std::fabs(T - want) < 1e-6, m);
    std::array<double, 1> p, v, a;
    traj.at_time(T, p, v, a);
    QT_CHECK(std::fabs(p[0] - 1.0) < 1e-9 && std::fabs(v[0]) < 1e-9 && std::fabs(a[0]) < 1e-9, "ends at 1 m, at rest");
    traj.at_time(T / 2, p, v, a);
    snprintf(m, sizeof m, "at T/2: %.6f m (0.5), v %.6f (0.5^(2/3) = %.6f)", p[0], v[0], std::pow(0.5, 2.0 / 3.0));
    QT_CHECK(std::fabs(p[0] - 0.5) < 1e-9 && std::fabs(v[0] - std::pow(0.5, 2.0 / 3.0)) < 1e-9, m);
    bool within = true;
    for (int i = 0; i <= 1000; i++) {
      traj.at_time(T * i / 1000.0, p, v, a);
      if (std::fabs(v[0]) > 1.0 + 1e-9 || std::fabs(a[0]) > 1.0 + 1e-9) within = false;
    }
    QT_CHECK(within, "velocity and acceleration stay within their limits");
  }
  {
    InputParameter<3> in;
    in.current_position = {0.0, 0.0, 0.5}; in.current_velocity = {0.0, 0.2, 0.0}; in.current_acceleration = {0.0, 0.0, 0.0};
    in.target_position = {5.0, -2.0, -3.5}; in.target_velocity = {0.0, -0.5, -0.5}; in.target_acceleration = {0.0, 0.0, 0.5};
    in.max_velocity = {3.0, 1.0, 3.0}; in.max_acceleration = {3.0, 2.0, 1.0}; in.max_jerk = {4.0, 3.0, 2.0};
    Ruckig<3> otg(0.01); OutputParameter<3> out;
    int steps = 0; Result r;
    while ((r = otg.update(in, out)) == Result::Working && steps < 100000) { out.pass_to_input(in); steps++; }
    QT_CHECK(r == Result::Finished, "online update() loop reaches the 3-DoF target");
    snprintf(m, sizeof m, "%d cycles of 10 ms for a %.4f s trajectory", steps, out.trajectory.get_duration());
    QT_CHECK(std::fabs(steps * 0.01 - out.trajectory.get_duration()) < 0.02, m);
    // The online loop stops on the first cycle past the end (up to 10 ms
    // further along, at the target velocity); the trajectory itself ends at
    // exactly the target state.
    std::array<double, 3> p, v, a;
    out.trajectory.at_time(out.trajectory.get_duration(), p, v, a);
    snprintf(m, sizeof m, "end state (%.6f, %.6f, %.6f), v (%.3f, %.3f, %.3f), a3 %.3f: expected (5, -2, -3.5), (0, -0.5, -0.5), 0.5",
             p[0], p[1], p[2], v[0], v[1], v[2], a[2]);
    QT_CHECK(std::fabs(p[0] - 5.0) < 1e-6 && std::fabs(p[1] + 2.0) < 1e-6 && std::fabs(p[2] + 3.5) < 1e-6 &&
             std::fabs(v[0]) < 1e-6 && std::fabs(v[1] + 0.5) < 1e-6 && std::fabs(v[2] + 0.5) < 1e-6 && std::fabs(a[2] - 0.5) < 1e-6, m);
  }
  QT_END();
}
