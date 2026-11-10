// ompl: geometric planning in the unit square around a wall. The wall
// x in [0.4, 0.6], y in [0, 0.8] separates start (0.1, 0.1) from goal
// (0.9, 0.1); the only way is over its top. RRTConnect must find an exact
// solution, every state of the (interpolated) path must be valid, the path
// must cross above y = 0.8 and be at least as long as the shortest way round
// (2*|(0.3, 0.7)| + 0.2 = 1.7232). Simplification must keep it valid and not
// make it longer. Without the gap (wall up to y = 1) no exact solution exists.
#include "common.h"
#include <cmath>
#include <ompl/base/spaces/RealVectorStateSpace.h>
#include <ompl/geometric/SimpleSetup.h>
#include <ompl/geometric/planners/rrt/RRTConnect.h>
namespace ob = ompl::base; namespace og = ompl::geometric;

static double wall_top = 0.8;
static bool valid(const ob::State *s) {
  const double *v = s->as<ob::RealVectorStateSpace::StateType>()->values;
  return !(v[0] >= 0.4 && v[0] <= 0.6 && v[1] <= wall_top);
}

static og::SimpleSetupPtr setup() {
  auto space = std::make_shared<ob::RealVectorStateSpace>(2);
  space->setBounds(0.0, 1.0);
  auto ss = std::make_shared<og::SimpleSetup>(space);
  ss->setStateValidityChecker(valid);
  ss->getSpaceInformation()->setStateValidityCheckingResolution(0.001);
  ob::ScopedState<> start(space), goal(space);
  start[0] = 0.1; start[1] = 0.1; goal[0] = 0.9; goal[1] = 0.1;
  ss->setStartAndGoalStates(start, goal);
  ss->setPlanner(std::make_shared<og::RRTConnect>(ss->getSpaceInformation()));
  return ss;
}

int main() {
  QT_BEGIN("ompl");
  ompl::msg::setLogLevel(ompl::msg::LOG_WARN);
  ompl::RNG::setSeed(42);
  char m[160];
  auto ss = setup();
  ob::PlannerStatus st = ss->solve(10.0);
  QT_CHECK(st == ob::PlannerStatus::EXACT_SOLUTION, "RRTConnect finds an exact solution");
  if (st == ob::PlannerStatus::EXACT_SOLUTION) {
    og::PathGeometric p = ss->getSolutionPath();
    double len0 = p.length();
    p.interpolate(2000);
    bool all = true, over = false;
    for (auto *s : p.getStates()) {
      if (!valid(s)) all = false;
      if (s->as<ob::RealVectorStateSpace::StateType>()->values[1] > 0.8) over = true;
    }
    QT_CHECK(all, "every state of the interpolated path is collision-free");
    QT_CHECK(over, "the path goes over the top of the wall");
    snprintf(m, sizeof m, "length %.4f >= shortest way round 1.7232", len0);
    QT_CHECK(len0 >= 1.7232 - 1e-3, m);
    ss->simplifySolution();
    og::PathGeometric q = ss->getSolutionPath();
    snprintf(m, sizeof m, "simplified: valid, length %.4f <= %.4f, >= 1.7232", q.length(), len0);
    QT_CHECK(q.check() && q.length() <= len0 + 1e-9 && q.length() >= 1.7232 - 1e-3, m);
  }
  wall_top = 1.0;
  auto ss2 = setup();
  ob::PlannerStatus st2 = ss2->solve(1.0);
  QT_CHECK(st2 != ob::PlannerStatus::EXACT_SOLUTION, "with the wall closed there is no exact solution");
  QT_END();
}
