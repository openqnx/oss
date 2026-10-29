// FCL (+libccd): collision and distance between two boxes.
#include "common.h"
#include <fcl/fcl.h>
int main() {
  QT_BEGIN("fcl");
  auto b = std::make_shared<fcl::Boxd>(1, 1, 1);
  fcl::Transform3d ta = fcl::Transform3d::Identity(), tb = fcl::Transform3d::Identity();
  tb.translation() << 0.5, 0, 0;
  fcl::CollisionObjectd a(b, ta), c(b, tb);
  fcl::CollisionRequestd rq; fcl::CollisionResultd rs;
  QT_CHECK(fcl::collide(&a, &c, rq, rs) > 0, "overlapping boxes collide");
  tb.translation() << 3, 0, 0; c.setTransform(tb);
  fcl::DistanceRequestd dq; fcl::DistanceResultd dr; fcl::distance(&a, &c, dq, dr);
  QT_CHECK(std::fabs(dr.min_distance - 2.0) < 1e-6, "distance is 2");
  QT_END();
}
