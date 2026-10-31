// urdfdom_headers (header-only URDF model types): vectors, rotations and
// colours parsed from URDF attribute strings, quaternion <-> roll/pitch/yaw,
// rotating a vector, composing rotations, parse errors, and a small model
// (two links, a revolute joint with limits) navigated like a parser's user.
#include "common.h"
#include <urdf_model/model.h>
#include <urdf_model/pose.h>
#include <urdf_model/color.h>
#include <urdf_exception/exception.h>
#include <cmath>
#include <memory>

static bool near(double a, double b) { return std::fabs(a - b) < 1e-9; }

int main() {
  QT_BEGIN("urdfdom_headers");
  urdf::Vector3 v; v.init("1 2.5 -3");
  QT_CHECK(near(v.x, 1) && near(v.y, 2.5) && near(v.z, -3), "Vector3::init(\"1 2.5 -3\")");
  bool threw = false;
  try { urdf::Vector3 w; w.init("1 x 3"); } catch (const urdf::ParseError &) { threw = true; }
  QT_CHECK(threw, "Vector3::init rejects \"1 x 3\" with ParseError");

  urdf::Rotation r; r.setFromRPY(0, 0, M_PI / 2);
  double qx, qy, qz, qw; r.getQuaternion(qx, qy, qz, qw);
  QT_CHECK(near(qx, 0) && near(qy, 0) && near(qz, std::sqrt(0.5)) && near(qw, std::sqrt(0.5)), "RPY(0,0,pi/2) -> quaternion (0,0,0.7071,0.7071)");
  urdf::Vector3 x(1, 0, 0), y = r * x;
  QT_CHECK(near(y.x, 0) && near(y.y, 1) && near(y.z, 0), "rotating (1,0,0) by yaw pi/2 gives (0,1,0)");
  urdf::Rotation r2 = r * r; double rr, pp, yy; r2.getRPY(rr, pp, yy);
  QT_CHECK(near(std::fabs(yy), M_PI) && near(rr, 0) && near(pp, 0), "two quarter turns compose to yaw pi");
  urdf::Rotation r3; r3.setFromRPY(0.1, -0.2, 0.3); r3.getRPY(rr, pp, yy);
  QT_CHECK(near(rr, 0.1) && near(pp, -0.2) && near(yy, 0.3), "RPY round trip (0.1,-0.2,0.3)");
  urdf::Rotation inv = r3.GetInverse(); urdf::Rotation id = r3 * inv;
  QT_CHECK(near(id.w, 1) || near(id.w, -1), "rotation times its inverse is identity");

  urdf::Color c;
  QT_CHECK(c.init("1 0 0.5 1") && c.r == 1.0f && c.g == 0.0f && c.b == 0.5f && c.a == 1.0f, "Color::init(\"1 0 0.5 1\")");
  QT_CHECK(!c.init("1 0"), "Color::init rejects 2 components");

  urdf::ModelInterface m;
  m.name_ = "arm";
  auto base = std::make_shared<urdf::Link>(); base->name = "base";
  auto tip = std::make_shared<urdf::Link>(); tip->name = "tip";
  auto j = std::make_shared<urdf::Joint>(); j->name = "elbow"; j->type = urdf::Joint::REVOLUTE;
  j->parent_link_name = "base"; j->child_link_name = "tip";
  j->axis = urdf::Vector3(0, 0, 1);
  j->parent_to_joint_origin_transform.position = urdf::Vector3(0, 0, 0.5);
  j->limits = std::make_shared<urdf::JointLimits>(); j->limits->lower = -1.57; j->limits->upper = 1.57; j->limits->effort = 10; j->limits->velocity = 2;
  m.links_["base"] = base; m.links_["tip"] = tip; m.joints_["elbow"] = j;
  tip->parent_joint = j; tip->setParent(base); base->child_links.push_back(tip); base->child_joints.push_back(j);
  m.root_link_ = base;
  QT_CHECK(m.getRoot() && m.getRoot()->name == "base", "model root is base");
  QT_CHECK(m.getLink("tip") && m.getLink("tip")->getParent()->name == "base", "tip's parent is base");
  auto jj = m.getJoint("elbow");
  QT_CHECK(jj && jj->type == urdf::Joint::REVOLUTE && near(jj->limits->upper - jj->limits->lower, 3.14), "elbow: revolute, range 3.14 rad");
  QT_CHECK(!m.getLink("nope") && m.getName() == "arm", "unknown link -> null; model name");
  std::vector<urdf::LinkSharedPtr> links; m.getLinks(links);
  QT_CHECK(links.size() == 2, "getLinks: 2");
  QT_END();
}
