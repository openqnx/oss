// pinocchio: rigid body dynamics on a planar double pendulum whose answers are
// known in closed form.  The pendulum hangs along -Z, both joints turn about
// Y, each link carries a point mass at its end (plus, in the URDF variant, a
// rotational inertia about Y):
//   - forward kinematics: joint and tip placements,
//   - the tip frame's Jacobian (LOCAL_WORLD_ALIGNED),
//   - CRBA: the joint space inertia matrix,
//   - RNEA: gravity, Coriolis/centrifugal and inertial torques,
//   - ABA: forward dynamics inverts RNEA,
//   - centre of mass, kinetic and potential energy,
//   - a free-flyer joint: neutral configuration and integration on SE(3),
//   - the same pendulum parsed from a URDF string (urdfdom): names, limits,
//     frames, and the same dynamics with the extra rotational inertia.
#include "common.h"
#include <pinocchio/multibody/model.hpp>
#include <pinocchio/multibody/data.hpp>
#include <pinocchio/algorithm/joint-configuration.hpp>
#include <pinocchio/algorithm/kinematics.hpp>
#include <pinocchio/algorithm/frames.hpp>
#include <pinocchio/algorithm/jacobian.hpp>
#include <pinocchio/algorithm/crba.hpp>
#include <pinocchio/algorithm/rnea.hpp>
#include <pinocchio/algorithm/aba.hpp>
#include <pinocchio/algorithm/center-of-mass.hpp>
#include <pinocchio/algorithm/energy.hpp>
#include <pinocchio/parsers/urdf.hpp>
#include <cmath>
#include <stdexcept>
#include <string>

using pinocchio::Model;
using pinocchio::Data;
using pinocchio::SE3;
using pinocchio::Inertia;
using Eigen::Vector2d;
using Eigen::Vector3d;
using Eigen::VectorXd;
using Eigen::Matrix2d;
using Eigen::Matrix3d;

static const double G = 9.81;
static const double L1 = 0.8, L2 = 0.5, M1 = 2.0, M2 = 1.5;

static bool near(double a, double b, double eps = 1e-9) { return std::fabs(a - b) < eps; }
static bool vnear(const Vector3d &a, const Vector3d &b, double eps = 1e-9) { return (a - b).norm() < eps; }

// Closed-form double pendulum.  Iy1/Iy2: rotational inertia of each link
// about Y at its centre of mass (0 for point masses).
struct Analytic {
  double iy1, iy2;
  Matrix2d M(const Vector2d &q) const {
    const double c2 = std::cos(q[1]);
    Matrix2d m;
    m(0, 0) = (M1 + M2) * L1 * L1 + M2 * L2 * L2 + 2 * M2 * L1 * L2 * c2 + iy1 + iy2;
    m(0, 1) = m(1, 0) = M2 * L2 * L2 + M2 * L1 * L2 * c2 + iy2;
    m(1, 1) = M2 * L2 * L2 + iy2;
    return m;
  }
  Vector2d coriolis(const Vector2d &q, const Vector2d &v) const {
    const double h = M2 * L1 * L2 * std::sin(q[1]);
    return Vector2d(-h * (2 * v[0] * v[1] + v[1] * v[1]), h * v[0] * v[0]);
  }
  Vector2d gravity(const Vector2d &q) const {
    const double s1 = std::sin(q[0]), s12 = std::sin(q[0] + q[1]);
    return Vector2d((M1 + M2) * G * L1 * s1 + M2 * G * L2 * s12, M2 * G * L2 * s12);
  }
};

static Vector3d p1(const Vector2d &q) { return Vector3d(-L1 * std::sin(q[0]), 0, -L1 * std::cos(q[0])); }
static Vector3d p2(const Vector2d &q) {
  return p1(q) + Vector3d(-L2 * std::sin(q[0] + q[1]), 0, -L2 * std::cos(q[0] + q[1]));
}

static Model build_pendulum() {
  Model model;
  model.gravity.linear(Vector3d(0, 0, -G));
  const pinocchio::JointIndex j1 =
      model.addJoint(0, pinocchio::JointModelRY(), SE3::Identity(), "shoulder");
  model.appendBodyToJoint(j1, Inertia(M1, Vector3d(0, 0, -L1), Matrix3d::Zero()), SE3::Identity());
  const pinocchio::JointIndex j2 =
      model.addJoint(j1, pinocchio::JointModelRY(), SE3(Matrix3d::Identity(), Vector3d(0, 0, -L1)), "elbow");
  model.appendBodyToJoint(j2, Inertia(M2, Vector3d(0, 0, -L2), Matrix3d::Zero()), SE3::Identity());
  model.addFrame(pinocchio::Frame("tip", j2, SE3(Matrix3d::Identity(), Vector3d(0, 0, -L2)),
                                  pinocchio::OP_FRAME));
  return model;
}

static const char *kUrdf = R"(<?xml version="1.0"?>
<robot name="double_pendulum">
  <link name="base"/>
  <link name="upper">
    <inertial>
      <origin xyz="0 0 -0.8"/>
      <mass value="2.0"/>
      <inertia ixx="0.01" ixy="0" ixz="0" iyy="0.04" iyz="0" izz="0.01"/>
    </inertial>
  </link>
  <link name="lower">
    <inertial>
      <origin xyz="0 0 -0.5"/>
      <mass value="1.5"/>
      <inertia ixx="0.01" ixy="0" ixz="0" iyy="0.02" iyz="0" izz="0.01"/>
    </inertial>
  </link>
  <link name="tip"/>
  <joint name="shoulder" type="revolute">
    <parent link="base"/><child link="upper"/>
    <origin xyz="0 0 0"/>
    <axis xyz="0 1 0"/>
    <limit lower="-1.5" upper="2.5" effort="30" velocity="4"/>
  </joint>
  <joint name="elbow" type="revolute">
    <parent link="upper"/><child link="lower"/>
    <origin xyz="0 0 -0.8"/>
    <axis xyz="0 1 0"/>
    <limit lower="-2" upper="2" effort="12" velocity="6"/>
  </joint>
  <joint name="tip_fixed" type="fixed">
    <parent link="lower"/><child link="tip"/>
    <origin xyz="0 0 -0.5"/>
  </joint>
</robot>
)";

// Dynamics of `model` at (q, v, a) against the closed form.
static void check_dynamics(const Model &model, const Analytic &an, const char *tag) {
  Data data(model);
  const Vector2d q(0.3, -0.7), v(0.9, -1.4), a(0.5, 2.0);
  std::string t(tag);

  pinocchio::crba(model, data, q);
  data.M.triangularView<Eigen::StrictlyLower>() = data.M.transpose().triangularView<Eigen::StrictlyLower>();
  QT_CHECK((data.M - an.M(q)).norm() < 1e-9, (t + ": CRBA inertia matrix").c_str());

  const Vector2d g = pinocchio::computeGeneralizedGravity(model, data, q);
  QT_CHECK((g - an.gravity(q)).norm() < 1e-9, (t + ": gravity torques").c_str());

  const Vector2d tau0 = pinocchio::rnea(model, data, q, v, Vector2d::Zero().eval());
  QT_CHECK((tau0 - an.coriolis(q, v) - an.gravity(q)).norm() < 1e-9,
           (t + ": RNEA Coriolis + gravity").c_str());

  const Vector2d tau = pinocchio::rnea(model, data, q, v, a);
  const Vector2d want = an.M(q) * a + an.coriolis(q, v) + an.gravity(q);
  QT_CHECK((tau - want).norm() < 1e-9, (t + ": RNEA full inverse dynamics").c_str());

  const Vector2d acc = pinocchio::aba(model, data, q, v, tau, pinocchio::Convention::WORLD);
  QT_CHECK((acc - a).norm() < 1e-8, (t + ": ABA inverts RNEA").c_str());

  const double ke = pinocchio::computeKineticEnergy(model, data, q, v);
  QT_CHECK(near(ke, 0.5 * v.dot(an.M(q) * v)), (t + ": kinetic energy").c_str());
}

int main() {
  QT_BEGIN("pinocchio");

  // ---- model built through the API ----------------------------------------
  Model model = build_pendulum();
  QT_CHECK(model.nq == 2 && model.nv == 2 && model.njoints == 3, "model dimensions");
  QT_CHECK(model.existJointName("elbow") && model.getJointId("elbow") == 2, "joint lookup by name");
  QT_CHECK(model.names[1] == "shoulder" && model.parents[2] == 1, "kinematic tree");
  QT_CHECK(near(pinocchio::computeTotalMass(model), M1 + M2), "total mass");

  Data data(model);
  const Vector2d q(0.3, -0.7);

  // Forward kinematics.
  pinocchio::forwardKinematics(model, data, q);
  pinocchio::updateFramePlacements(model, data);
  const pinocchio::FrameIndex tip = model.getFrameId("tip");
  QT_CHECK(tip < static_cast<pinocchio::FrameIndex>(model.nframes), "tip frame exists");
  QT_CHECK(vnear(data.oMi[1].translation(), Vector3d::Zero()), "FK: shoulder at the origin");
  QT_CHECK(vnear(data.oMi[2].translation(), p1(q)), "FK: elbow position");
  QT_CHECK(vnear(data.oMf[tip].translation(), p2(q)), "FK: tip position");
  {
    // The tip frame is rotated by q1 + q2 about Y.
    const Matrix3d R = Eigen::AngleAxisd(q[0] + q[1], Vector3d::UnitY()).toRotationMatrix();
    QT_CHECK((data.oMf[tip].rotation() - R).norm() < 1e-12, "FK: tip orientation");
  }
  {
    // Straight down at q = 0, horizontal at q1 = pi/2.
    Data d(model);
    pinocchio::framesForwardKinematics(model, d, Vector2d::Zero().eval());
    QT_CHECK(vnear(d.oMf[tip].translation(), Vector3d(0, 0, -(L1 + L2))), "FK: q = 0 hangs straight down");
    pinocchio::framesForwardKinematics(model, d, Vector2d(M_PI / 2, 0));
    QT_CHECK(vnear(d.oMf[tip].translation(), Vector3d(-(L1 + L2), 0, 0), 1e-12), "FK: q1 = pi/2 is horizontal");
  }

  // Jacobian of the tip, world aligned: rows vx vy vz wx wy wz.
  {
    pinocchio::Data::Matrix6x J(6, model.nv);
    J.setZero();
    pinocchio::computeFrameJacobian(model, data, q, tip, pinocchio::LOCAL_WORLD_ALIGNED, J);
    const double c1 = std::cos(q[0]), s1 = std::sin(q[0]);
    const double c12 = std::cos(q[0] + q[1]), s12 = std::sin(q[0] + q[1]);
    Eigen::Matrix<double, 6, 2> W;
    W << -L1 * c1 - L2 * c12, -L2 * c12,
         0, 0,
         L1 * s1 + L2 * s12, L2 * s12,
         0, 0,
         1, 1,
         0, 0;
    QT_CHECK((J - W).norm() < 1e-12, "tip Jacobian (LOCAL_WORLD_ALIGNED)");

    // The Jacobian maps joint velocity to the frame velocity FK computes.
    const Vector2d v(0.9, -1.4);
    pinocchio::forwardKinematics(model, data, q, v);
    const pinocchio::Motion vel = pinocchio::getFrameVelocity(model, data, tip, pinocchio::LOCAL_WORLD_ALIGNED);
    QT_CHECK((vel.toVector() - J * v).norm() < 1e-12, "frame velocity = J * v");
  }

  // Centre of mass and potential energy.
  {
    const Vector3d com = pinocchio::centerOfMass(model, data, q);
    const Vector3d want = (M1 * p1(q) + M2 * p2(q)) / (M1 + M2);
    QT_CHECK(vnear(com, want), "centre of mass");
    const double pe = pinocchio::computePotentialEnergy(model, data, q);
    QT_CHECK(near(pe, (M1 + M2) * G * want[2]), "potential energy");
  }

  check_dynamics(model, Analytic{0.0, 0.0}, "point masses");

  // Hanging at rest needs no torque; held horizontal it needs the full moment.
  {
    Data d(model);
    const Vector2d z = Vector2d::Zero();
    QT_CHECK(pinocchio::rnea(model, d, z, z, z).norm() < 1e-12, "RNEA: no torque hanging at rest");
    const Vector2d t = pinocchio::rnea(model, d, Vector2d(M_PI / 2, 0), z, z);
    QT_CHECK(near(t[0], M1 * G * L1 + M2 * G * (L1 + L2)) && near(t[1], M2 * G * L2),
             "RNEA: static torque held horizontal");
  }

  // ---- configuration space of a free-flyer joint ---------------------------
  {
    Model ff;
    ff.addJoint(0, pinocchio::JointModelFreeFlyer(), SE3::Identity(), "root");
    QT_CHECK(ff.nq == 7 && ff.nv == 6, "free flyer: nq = 7, nv = 6");
    const VectorXd q0 = pinocchio::neutral(ff);
    VectorXd want(7);
    want << 0, 0, 0, 0, 0, 0, 1;
    QT_CHECK((q0 - want).norm() < 1e-15, "free flyer: neutral configuration");
    VectorXd v(6);
    v << 1, 2, 3, 0, 0, 0;
    VectorXd qt = pinocchio::integrate(ff, q0, v);
    want << 1, 2, 3, 0, 0, 0, 1;
    QT_CHECK((qt - want).norm() < 1e-12, "free flyer: integrate a translation");
    v << 0, 0, 0, 0, 0, M_PI / 2;
    VectorXd qr = pinocchio::integrate(ff, q0, v);
    want << 0, 0, 0, 0, 0, std::sin(M_PI / 4), std::cos(M_PI / 4);
    QT_CHECK((qr - want).norm() < 1e-12, "free flyer: integrate a quarter turn about Z");
    const VectorXd back = pinocchio::difference(ff, q0, qr);
    QT_CHECK((back - v).norm() < 1e-12, "free flyer: difference inverts integrate");
  }

  // ---- the same pendulum from URDF -----------------------------------------
  {
    Model um;
    bool parsed = true;
    try {
      pinocchio::urdf::buildModelFromXML(std::string(kUrdf), um);
    } catch (const std::exception &e) {
      printf("  exception: %s\n", e.what());
      parsed = false;
    }
    QT_CHECK(parsed, "URDF: string parsed");
    if (parsed) {
      um.gravity.linear(Vector3d(0, 0, -G));
      QT_CHECK(um.name == "double_pendulum", "URDF: robot name");
      QT_CHECK(um.nq == 2 && um.nv == 2 && um.njoints == 3, "URDF: dimensions");
      QT_CHECK(um.existJointName("shoulder") && um.existJointName("elbow") &&
               um.getJointId("shoulder") == 1 && um.getJointId("elbow") == 2, "URDF: joint names");
      QT_CHECK(near(um.lowerPositionLimit[0], -1.5) && near(um.upperPositionLimit[0], 2.5) &&
               near(um.lowerPositionLimit[1], -2.0) && near(um.upperPositionLimit[1], 2.0),
               "URDF: position limits");
      QT_CHECK(near(um.effortLimit[0], 30) && near(um.effortLimit[1], 12) &&
               near(um.velocityLimit[0], 4) && near(um.velocityLimit[1], 6), "URDF: effort and velocity limits");
      QT_CHECK(um.existBodyName("upper") && um.existBodyName("lower") && um.existFrame("tip_fixed"),
               "URDF: link and fixed joint frames");
      QT_CHECK(near(um.inertias[1].mass(), M1) && near(um.inertias[2].mass(), M2) &&
               vnear(um.inertias[2].lever(), Vector3d(0, 0, -L2)), "URDF: link inertias");

      Data ud(um);
      pinocchio::framesForwardKinematics(um, ud, q);
      QT_CHECK(um.existFrame("tip") && vnear(ud.oMf[um.getFrameId("tip")].translation(), p2(q)),
               "URDF: tip position");
      check_dynamics(um, Analytic{0.04, 0.02}, "URDF");
    }

    bool threw = false;
    try {
      Model bad;
      pinocchio::urdf::buildModelFromXML(std::string("<robot name=\"x\"><link name=\"a\"/><oops"), bad);
    } catch (const std::exception &) {
      threw = true;
    }
    QT_CHECK(threw, "URDF: malformed input rejected");
  }

  QT_END();
}
