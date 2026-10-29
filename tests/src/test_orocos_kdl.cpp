// Orocos KDL: forward kinematics of a two-link planar arm.
#include "common.h"
#include <kdl/chain.hpp>
#include <kdl/chainfksolverpos_recursive.hpp>
#include <cmath>
using namespace KDL;
int main() {
  QT_BEGIN("orocos-kdl");
  Chain c;
  c.addSegment(Segment(Joint(Joint::RotZ), Frame(Vector(1, 0, 0))));
  c.addSegment(Segment(Joint(Joint::RotZ), Frame(Vector(1, 0, 0))));
  ChainFkSolverPos_recursive fk(c); JntArray q(2); q(0) = M_PI / 2; q(1) = 0; Frame f;
  QT_CHECK(fk.JntToCart(q, f) >= 0, "FK solves");
  QT_CHECK(std::fabs(f.p.x()) < 1e-9 && std::fabs(f.p.y() - 2) < 1e-9, "end effector at (0,2)");
  QT_END();
}
