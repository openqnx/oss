// sophus: Lie group operations with known answers -- SO(2)/SO(3) exp and
// log, the SE(3) exponential of a screw motion (a quarter circle), group
// action, composition and inverse, the adjoint identity, hat/vee,
// interpolation and averaging on the manifold, and Sim(3) scaling.
#include "common.h"
#include <sophus/average.hpp>
#include <sophus/interpolate.hpp>
#include <sophus/se2.hpp>
#include <sophus/se3.hpp>
#include <sophus/sim3.hpp>
#include <sophus/so2.hpp>
#include <sophus/so3.hpp>
#include <array>
#include <cmath>

using Eigen::Matrix3d;
using Eigen::Vector2d;
using Eigen::Vector3d;
typedef Eigen::Matrix<double, 6, 1> Vector6d;

static bool near(double a, double b, double eps = 1e-12) { return std::fabs(a - b) < eps; }
template <class A, class B> static bool vnear(const A &a, const B &b, double eps = 1e-12) {
  return (a - b).cwiseAbs().maxCoeff() < eps;
}

int main() {
  QT_BEGIN("sophus");
  const double pi = Sophus::Constants<double>::pi();

  // SO(2)
  Sophus::SO2d r30 = Sophus::SO2d::exp(pi / 6);
  QT_CHECK(near(r30.matrix()(0, 0), std::sqrt(3.0) / 2) && near(r30.matrix()(1, 0), 0.5) &&
           near(r30.matrix()(0, 1), -0.5), "SO2 exp(pi/6) is the 30 degree rotation matrix");
  QT_CHECK(near((r30 * r30 * r30).log(), pi / 2), "three 30 degree rotations have log pi/2");
  QT_CHECK(vnear(Sophus::SO2d::exp(pi / 2) * Vector2d(1, 0), Vector2d(0, 1)), "SO2 quarter turn maps x to y");

  // SO(3)
  Sophus::SO3d rz = Sophus::SO3d::exp(Vector3d(0, 0, pi / 2));
  QT_CHECK(vnear(rz * Vector3d(1, 0, 0), Vector3d(0, 1, 0)) && vnear(rz * Vector3d(0, 1, 0), Vector3d(-1, 0, 0)) &&
           vnear(rz * Vector3d(0, 0, 1), Vector3d(0, 0, 1)), "SO3 exp(0,0,pi/2) turns x to y, y to -x, keeps z");
  QT_CHECK(near(rz.unit_quaternion().w(), std::sqrt(0.5)) && near(rz.unit_quaternion().z(), std::sqrt(0.5)) &&
           near(rz.unit_quaternion().x(), 0) && near(rz.unit_quaternion().y(), 0), "its quaternion is (sqrt(1/2); 0, 0, sqrt(1/2))");
  Vector3d w(0.3, -0.7, 1.1);
  QT_CHECK(vnear(Sophus::SO3d::exp(w).log(), w), "SO3 log(exp(w)) = w");
  QT_CHECK(vnear((Sophus::SO3d::exp(w) * Sophus::SO3d::exp(-w)).matrix(), Matrix3d::Identity()), "exp(w) exp(-w) = I");
  Matrix3d R = Sophus::SO3d::exp(w).matrix();
  QT_CHECK(vnear(R * R.transpose(), Matrix3d::Identity()) && near(R.determinant(), 1.0), "exp(w) is orthonormal with determinant 1");
  // Rodrigues: rotating v about unit axis k by angle t
  {
    double t = w.norm(); Vector3d k = w / t, v(0.5, 2.0, -1.5);
    Vector3d rod = v * std::cos(t) + k.cross(v) * std::sin(t) + k * k.dot(v) * (1 - std::cos(t));
    QT_CHECK(vnear(Sophus::SO3d::exp(w) * v, rod), "SO3 action equals Rodrigues' formula");
  }
  QT_CHECK(vnear(Sophus::SO3d::vee(Sophus::SO3d::hat(w)), w) && near(Sophus::SO3d::hat(w)(2, 1), 0.3) &&
           near(Sophus::SO3d::hat(w)(1, 2), -0.3), "hat/vee: skew-symmetric matrix and back");
  QT_CHECK(vnear(Sophus::SO3d::rotX(pi).matrix().diagonal(), Vector3d(1, -1, -1)), "rotX(pi) = diag(1,-1,-1)");

  // SE(3)
  Sophus::SE3d T(rz, Vector3d(1, 2, 3));
  QT_CHECK(vnear(T * Vector3d(1, 0, 0), Vector3d(1, 3, 3)), "SE3 (Rz90, t=(1,2,3)) maps (1,0,0) to (1,3,3)");
  QT_CHECK(vnear(T.inverse() * Vector3d(1, 3, 3), Vector3d(1, 0, 0)), "its inverse maps it back");
  QT_CHECK(vnear((T * T.inverse()).matrix(), Eigen::Matrix4d::Identity()), "T T^-1 = I");
  QT_CHECK(vnear((T * T).translation(), Vector3d(-1, 3, 6)) && vnear((T * T).rotationMatrix().diagonal(), Vector3d(-1, -1, 1)),
           "T T: translation (-1,3,6), rotation pi about z");
  {
    // Constant twist: forward speed pi/2 while turning pi/2 about z, for unit
    // time -- a quarter of the unit circle, ending at (1,1,0) facing +y.
    Vector6d xi; xi << pi / 2, 0, 0, 0, 0, pi / 2;
    Sophus::SE3d arc = Sophus::SE3d::exp(xi);
    QT_CHECK(vnear(arc.translation(), Vector3d(1, 1, 0)) && vnear(arc.so3() * Vector3d(1, 0, 0), Vector3d(0, 1, 0)),
             "SE3 exp of a screw twist is a quarter circle ending at (1,1,0)");
    QT_CHECK(vnear(arc.log(), xi), "SE3 log(exp(xi)) = xi");
    Vector6d tr; tr << 0.5, -1, 2, 0, 0, 0;
    QT_CHECK(vnear(Sophus::SE3d::exp(tr).translation(), Vector3d(0.5, -1, 2)) &&
             vnear(Sophus::SE3d::exp(tr).rotationMatrix(), Matrix3d::Identity()), "pure translation twist");
    // Adjoint: T exp(a) T^-1 = exp(Adj(T) a)
    Vector6d a; a << 0.1, 0.2, -0.3, 0.4, -0.5, 0.6;
    Sophus::SE3d lhs = T * Sophus::SE3d::exp(a) * T.inverse();
    Vector6d adj = T.Adj() * a;
    Sophus::SE3d rhs = Sophus::SE3d::exp(adj);
    QT_CHECK(vnear(lhs.matrix(), rhs.matrix(), 1e-10), "adjoint identity T exp(a) T^-1 = exp(Adj(T) a)");
  }
  {
    Sophus::SE3d A, B(Sophus::SO3d::exp(Vector3d(0, 0, pi / 2)), Vector3d(2, 0, 0));
    Sophus::SE3d M = Sophus::interpolate(A, B, 0.5);
    QT_CHECK(vnear(M.so3().log(), Vector3d(0, 0, pi / 4)), "interpolation halfway turns pi/4");
    QT_CHECK(vnear((M * M).matrix(), B.matrix(), 1e-10), "the halfway pose applied twice gives the end pose");
    QT_CHECK(vnear(Sophus::interpolate(A, B, 0.0).matrix(), A.matrix()) && vnear(Sophus::interpolate(A, B, 1.0).matrix(), B.matrix(), 1e-10),
             "interpolation end points");
  }
  {
    std::array<Sophus::SO3d, 3> rs = {Sophus::SO3d::exp(Vector3d(0, 0, 0.2)), Sophus::SO3d::exp(Vector3d(0, 0, 0.5)),
                                      Sophus::SO3d::exp(Vector3d(0, 0, 0.8))};
    // (the quaternion mean; for angles symmetric about 0.5 it is 0.5)
    auto avg = Sophus::average(rs);
    QT_CHECK(bool(avg) && vnear(avg->log(), Vector3d(0, 0, 0.5), 1e-9), "average of rotations 0.2, 0.5, 0.8 about z is 0.5");
  }

  // SE(2)
  {
    Sophus::SE2d P(pi / 2, Vector2d(1, 0));
    QT_CHECK(vnear(P * Vector2d(1, 0), Vector2d(1, 1)), "SE2 (90 degrees, t=(1,0)) maps (1,0) to (1,1)");
    Vector3d xi(pi / 2, 0, pi / 2);   // (vx, vy, omega): the planar quarter circle
    QT_CHECK(vnear(Sophus::SE2d::exp(xi).translation(), Vector2d(1, 1)) && vnear(Sophus::SE2d::exp(xi).log(), xi),
             "SE2 exp of the arc twist ends at (1,1); log inverts it");
  }

  // Sim(3)
  {
    Sophus::Sim3d S(Sophus::RxSO3d(2.0, rz), Vector3d(0, 0, 1));
    QT_CHECK(near(S.scale(), 2.0) && vnear(S * Vector3d(1, 0, 0), Vector3d(0, 2, 1)), "Sim3 scale 2, Rz90, t=(0,0,1) maps (1,0,0) to (0,2,1)");
    QT_CHECK(vnear(S.inverse() * Vector3d(0, 2, 1), Vector3d(1, 0, 0)) && near((S * S).scale(), 4.0), "Sim3 inverse and composed scale 4");
    Eigen::Matrix<double, 7, 1> l = S.log();
    QT_CHECK(near(l(6), std::log(2.0)) && vnear(Sophus::Sim3d::exp(l).matrix(), S.matrix(), 1e-10), "Sim3 log holds ln 2; exp inverts it");
  }

  // float instantiation and the Eigen::Map interface
  {
    float raw[4] = {0, 0, 0, 1};   // quaternion x, y, z, w
    Eigen::Map<Sophus::SO3f> m(raw);
    m = Sophus::SO3f::rotZ(float(pi / 2)) * m;
    QT_CHECK(near(raw[2], std::sqrt(0.5f), 1e-6) && near(raw[3], std::sqrt(0.5f), 1e-6) && near(raw[0], 0, 1e-6),
             "Eigen::Map<SO3f> writes the product into the mapped quaternion");
  }
  QT_END();
}
