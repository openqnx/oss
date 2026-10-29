#include "common.h"
#include <Eigen/Dense>
#include <Eigen/Geometry>
int main() {
  QT_BEGIN("eigen");
  Eigen::Matrix3d a = Eigen::Matrix3d::Identity();
  Eigen::Matrix3d b = Eigen::Matrix3d::Constant(2.0);
  Eigen::Matrix3d c = a * b;
  QT_CHECK(c.isApprox(b), "identity * B == B");
  Eigen::Vector3d v(1, 2, 3);
  QT_CHECK(std::abs(v.norm() - std::sqrt(14.0)) < 1e-12, "vector norm");
  Eigen::Matrix2d m; m << 4, 7, 2, 6;
  Eigen::Matrix2d inv = m.inverse();
  QT_CHECK((m * inv).isApprox(Eigen::Matrix2d::Identity(), 1e-12), "matrix inverse");
  Eigen::Quaterniond q(Eigen::AngleAxisd(M_PI / 2, Eigen::Vector3d::UnitZ()));
  Eigen::Vector3d r = q * Eigen::Vector3d::UnitX();
  QT_CHECK(r.isApprox(Eigen::Vector3d::UnitY(), 1e-9), "quaternion rotation 90deg about Z");
  printf("  info eigen %d.%d.%d\n", EIGEN_WORLD_VERSION, EIGEN_MAJOR_VERSION, EIGEN_MINOR_VERSION);
  QT_END();
}
