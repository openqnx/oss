/* GTSAM: factor-graph SLAM with known answers -- a Pose2 square loop with
 * noisy odometry and a loop closure optimised by Levenberg-Marquardt back
 * onto the true poses, marginal covariances (the prior-anchored pose is the
 * most certain), Pose3/Rot3 group operations, and iSAM2 incremental updates
 * agreeing with the batch solution. */
#include "common.h"
#include <algorithm>
#include <cmath>
#include <gtsam/geometry/Pose2.h>
#include <gtsam/geometry/Pose3.h>
#include <gtsam/inference/Symbol.h>
#include <gtsam/nonlinear/NonlinearFactorGraph.h>
#include <gtsam/nonlinear/Values.h>
#include <gtsam/nonlinear/LevenbergMarquardtOptimizer.h>
#include <gtsam/nonlinear/Marginals.h>
#include <gtsam/nonlinear/ISAM2.h>
#include <gtsam/slam/BetweenFactor.h>
#include <gtsam/nonlinear/PriorFactor.h>

using namespace gtsam;

int main() {
  QT_BEGIN("gtsam");
  printf("  info GTSAM %s\n", GTSAM_VERSION_STRING);

  Pose2 a(1, 2, M_PI / 2), b(2, 0, 0);
  Pose2 c = a.compose(b);
  QT_CHECK(std::fabs(c.x() - 1) < 1e-12 && std::fabs(c.y() - 4) < 1e-12 && std::fabs(c.theta() - M_PI / 2) < 1e-12,
           "Pose2 compose");
  QT_CHECK(a.between(c).equals(b, 1e-12), "Pose2 between is the inverse of compose");
  Rot3 r = Rot3::Rz(M_PI / 2);
  Point3 p = r.rotate(Point3(1, 0, 0));
  QT_CHECK(p.isApprox(Point3(0, 1, 0), 1e-12), "Rot3::Rz(90 deg) maps x to y");
  Pose3 T(r, Point3(1, 2, 3));
  QT_CHECK(T.transformFrom(Point3(1, 0, 0)).isApprox(Point3(1, 3, 3), 1e-12) &&
           T.inverse().compose(T).equals(Pose3(), 1e-12), "Pose3 transform and inverse");
  Vector6 xi = Pose3::Logmap(T);
  QT_CHECK(Pose3::Expmap(xi).equals(T, 1e-9), "Pose3 Expmap(Logmap(T)) == T");

  NonlinearFactorGraph g;
  auto priorNoise = noiseModel::Diagonal::Sigmas(Vector3(0.01, 0.01, 0.01));
  auto odoNoise = noiseModel::Diagonal::Sigmas(Vector3(0.2, 0.2, 0.1));
  g.add(PriorFactor<Pose2>(1, Pose2(0, 0, 0), priorNoise));
  for (int i = 1; i <= 4; ++i) g.add(BetweenFactor<Pose2>(i, i + 1, Pose2(2, 0, M_PI / 2), odoNoise));
  g.add(BetweenFactor<Pose2>(5, 1, Pose2(0, 0, 0), odoNoise));   // loop closure: back at the start
  Values init;
  init.insert(1, Pose2(0.5, 0.0, 0.2));
  init.insert(2, Pose2(2.3, 0.1, -0.2));
  init.insert(3, Pose2(4.1, 0.1, M_PI / 2));
  init.insert(4, Pose2(4.0, 2.0, M_PI));
  init.insert(5, Pose2(2.1, 2.1, -M_PI / 2));
  double e0 = g.error(init);
  Values res = LevenbergMarquardtOptimizer(g, init).optimize();
  double e1 = g.error(res);
  printf("  info error %.4f -> %.3g\n", e0, e1);
  QT_CHECK(e1 < 1e-8 && e1 < e0, "LM drives the graph error to ~0");
  const Pose2 truth[5] = {Pose2(0, 0, 0), Pose2(2, 0, M_PI / 2), Pose2(2, 2, M_PI), Pose2(0, 2, -M_PI / 2), Pose2(0, 0, 0)};
  bool ok = true;
  for (int i = 0; i < 5; ++i) ok = ok && res.at<Pose2>(i + 1).equals(truth[i], 1e-5);
  QT_CHECK(ok, "optimised poses are the square's corners");

  Marginals m(g, res);
  Matrix c1 = m.marginalCovariance(1), c3 = m.marginalCovariance(3);
  printf("  info var(x1)=%.2e var(x3)=%.2e\n", c1(0, 0), c3(0, 0));
  QT_CHECK(c1(0, 0) < c3(0, 0) && c1(0, 0) > 0, "anchored pose 1 is more certain than pose 3");

  /* Relinearise on every update: with the default threshold (0.1) and skip
     (10) iSAM2 keeps the linearisation of the rough initial guesses and stops
     short of the batch optimum, which is what the first VM run showed. */
  ISAM2Params ip;
  ip.relinearizeThreshold = 0.0;
  ip.relinearizeSkip = 1;
  ISAM2 isam(ip);
  NonlinearFactorGraph step; Values sv;
  step.add(g[0]); step.add(g[1]); sv.insert(1, init.at<Pose2>(1)); sv.insert(2, init.at<Pose2>(2));
  isam.update(step, sv);
  for (int i = 2; i <= 4; ++i) {
    NonlinearFactorGraph s; Values v;
    s.add(g[i]);
    v.insert(i + 1, init.at<Pose2>(i + 1));
    if (i == 4) s.add(g[5]);
    isam.update(s, v);
  }
  for (int k = 0; k < 10; ++k) isam.update();
  Values inc = isam.calculateEstimate();
  ok = inc.size() == 5;
  double worst = 0;
  for (int i = 1; ok && i <= 5; ++i) {
    Vector3 d = Pose2::Logmap(res.at<Pose2>(i).between(inc.at<Pose2>(i)));
    worst = std::max(worst, d.cwiseAbs().maxCoeff());
  }
  printf("  info iSAM2 vs batch: largest difference %.2e\n", worst);
  ok = ok && worst < 1e-3;
  QT_CHECK(ok, "iSAM2 incremental estimate matches the batch solution");
  QT_END();
}
