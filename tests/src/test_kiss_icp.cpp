// kiss-icp: the C++ core with known answers -- voxel downsampling and the
// voxel hash map (nearest neighbour, per-voxel limits, range pruning), the
// preprocessor (range filter, deskewing), the adaptive threshold's formula,
// ICP registration recovering a known rigid transform, the whole odometry
// pipeline following a simulated sensor through a static scene, and the
// trajectory error metric.
#include "common.h"
#include <kiss_icp/core/Preprocessing.hpp>
#include <kiss_icp/core/Registration.hpp>
#include <kiss_icp/core/Threshold.hpp>
#include <kiss_icp/core/VoxelHashMap.hpp>
#include <kiss_icp/core/VoxelUtils.hpp>
#include <kiss_icp/metrics/Metrics.hpp>
#include <kiss_icp/pipeline/KissICP.hpp>
#include <cmath>
#include <cstdint>
#include <limits>
#include <set>
#include <tuple>
#include <vector>

using Eigen::Vector3d;
typedef std::vector<Vector3d> Cloud;

static bool vnear(const Vector3d &a, const Vector3d &b, double eps) { return (a - b).norm() < eps; }

// A regular lattice of (2n+1)^3 points with the given spacing, centred on c.
static Cloud lattice(int n, double spacing, const Vector3d &c = Vector3d::Zero()) {
  Cloud p;
  for (int i = -n; i <= n; i++) for (int j = -n; j <= n; j++) for (int k = -n; k <= n; k++)
    p.push_back(c + spacing * Vector3d(i, j, k));
  return p;
}

// The static scene of the pipeline test: floor, two walls and a few pillars
// of a 24 m x 16 m hall, sampled every 0.4 m with a small deterministic
// jitter so that no direction is left unconstrained and no lattice repeats.
static Cloud scene() {
  Cloud w; std::uint32_t s = 12345u;
  auto jit = [&]() { s = s * 1664525u + 1013904223u; return ((s >> 8) & 0xffff) / 65535.0 * 0.1 - 0.05; };
  for (double x = -12; x <= 12; x += 0.4) for (double y = -8; y <= 8; y += 0.4) w.push_back(Vector3d(x + jit(), y + jit(), -1.5));
  for (double x = -12; x <= 12; x += 0.4) for (double z = -1.5; z <= 3; z += 0.4) {
    w.push_back(Vector3d(x + jit(), -8, z + jit()));
    w.push_back(Vector3d(x + jit(), 8, z + jit()));
  }
  for (double y = -8; y <= 8; y += 0.4) for (double z = -1.5; z <= 3; z += 0.4) {
    w.push_back(Vector3d(-12, y + jit(), z + jit()));
    w.push_back(Vector3d(12, y + jit(), z + jit()));
  }
  const double px[4] = {-6, -2, 3, 7}, py[4] = {3, -4, 5, -2};
  for (int p = 0; p < 4; p++) for (double z = -1.5; z <= 3; z += 0.2) for (int a = 0; a < 12; a++)
    w.push_back(Vector3d(px[p] + 0.3 * std::cos(a * 0.5236), py[p] + 0.3 * std::sin(a * 0.5236), z));
  return w;
}

int main() {
  QT_BEGIN("kiss-icp");

  // ---- voxels ----
  QT_CHECK(kiss_icp::PointToVoxel(Vector3d(-0.1, 0.0, 2.5), 1.0) == kiss_icp::Voxel(-1, 0, 2) &&
           kiss_icp::PointToVoxel(Vector3d(1.99, -2.0, 0.49), 0.5) == kiss_icp::Voxel(3, -4, 0),
           "PointToVoxel floors towards minus infinity");
  {
    Cloud g;   // 4 x 4 x 4 points, 0.5 apart, in [0,2)^3: 8 voxels of size 1
    for (int i = 0; i < 4; i++) for (int j = 0; j < 4; j++) for (int k = 0; k < 4; k++) g.push_back(Vector3d(0.25 + 0.5 * i, 0.25 + 0.5 * j, 0.25 + 0.5 * k));
    Cloud d = kiss_icp::VoxelDownsample(g, 1.0);
    std::set<std::tuple<int, int, int>> vox;
    for (const auto &p : d) { auto v = kiss_icp::PointToVoxel(p, 1.0); vox.insert(std::make_tuple(v.x(), v.y(), v.z())); }
    QT_CHECK(d.size() == 8 && vox.size() == 8, "VoxelDownsample keeps one point in each of the 8 voxels");
    QT_CHECK(kiss_icp::VoxelDownsample(g, 0.5).size() == 64 && kiss_icp::VoxelDownsample(g, 4.0).size() == 1,
             "a voxel per point keeps all 64; one big voxel keeps 1");
  }

  // ---- voxel hash map ----
  {
    kiss_icp::VoxelHashMap map(1.0, 100.0, 20);
    QT_CHECK(map.Empty(), "a new map is empty");
    Cloud l = lattice(4, 0.5);   // 729 points, 0.5 apart
    map.AddPoints(l);
    QT_CHECK(!map.Empty() && map.Pointcloud().size() == 729, "729 lattice points stored");
    map.AddPoints(l);
    map.AddPoints({Vector3d(0.1, 0.0, 0.0)});   // closer than voxel_size/sqrt(20) = 0.2236 to (0,0,0)
    QT_CHECK(map.Pointcloud().size() == 729, "duplicates and points closer than the map resolution are not added");
    auto [nn, dist] = map.GetClosestNeighbor(Vector3d(1.5 + 0.1, -0.5 + 0.05, 1.0 - 0.02));
    QT_CHECK(vnear(nn, Vector3d(1.5, -0.5, 1.0), 1e-12) && std::fabs(dist - std::sqrt(0.01 + 0.0025 + 0.0004)) < 1e-12,
             "closest neighbour of a displaced lattice point is that point, at the displacement's length");
    auto [far, fdist] = map.GetClosestNeighbor(Vector3d(50, 50, 50));
    (void)far;
    QT_CHECK(fdist == std::numeric_limits<double>::max(), "no neighbour in the 27 voxels around a far query");

    kiss_icp::VoxelHashMap small(1.0, 5.0, 3);
    Cloud corners;   // the 8 corners of one voxel, 0.9 apart: more than the limit of 3
    for (int i = 0; i < 2; i++) for (int j = 0; j < 2; j++) for (int k = 0; k < 2; k++) corners.push_back(Vector3d(0.05 + 0.9 * i, 0.05 + 0.9 * j, 0.05 + 0.9 * k));
    small.AddPoints(corners);
    QT_CHECK(small.Pointcloud().size() == 3, "max_points_per_voxel limits a voxel to 3 points");
    small.Clear();
    small.Update(Cloud{Vector3d(1.2, 0.3, 0.3), Vector3d(9.2, 0.3, 0.3)}, Vector3d(0, 0, 0));
    Cloud left = small.Pointcloud();
    QT_CHECK(left.size() == 1 && vnear(left[0], Vector3d(1.2, 0.3, 0.3), 1e-12), "Update drops the voxel beyond max_distance 5 from the origin");
    Sophus::SE3d shift(Sophus::SO3d(), Vector3d(10, 0, 0));
    small.Update(Cloud{Vector3d(0.5, 0.5, 0.5)}, shift);   // stored at (10.5, 0.5, 0.5); the old point is now 8.8 away
    left = small.Pointcloud();
    QT_CHECK(left.size() == 1 && vnear(left[0], Vector3d(10.5, 0.5, 0.5), 1e-12), "Update with a pose transforms the points and prunes around the new position");
  }

  // ---- preprocessing ----
  {
    kiss_icp::Preprocessor range_only(10.0, 1.0, false, 2);
    Cloud in = {Vector3d(0.5, 0, 0), Vector3d(0, 5, 0), Vector3d(0, 0, 20), Vector3d(3, 4, 0)};
    Cloud out = range_only.Preprocess(in, {}, Sophus::SE3d());
    QT_CHECK(out.size() == 2 && vnear(out[0], Vector3d(0, 5, 0), 1e-12) && vnear(out[1], Vector3d(3, 4, 0), 1e-12),
             "range filter keeps the points between 1 m and 10 m");
    // Deskewing: the sensor moved 1 m along x during the sweep; a point measured
    // at relative time s is moved by (s - 1) of that motion.
    kiss_icp::Preprocessor deskew(100.0, 0.0, true, 2);
    Cloud sweep = {Vector3d(5, 1, 0), Vector3d(5, 2, 0), Vector3d(5, 3, 0)};
    Cloud ds = deskew.Preprocess(sweep, {10.0, 10.05, 10.1}, Sophus::SE3d(Sophus::SO3d(), Vector3d(1, 0, 0)));
    QT_CHECK(ds.size() == 3 && vnear(ds[0], Vector3d(4, 1, 0), 1e-9) && vnear(ds[1], Vector3d(4.5, 2, 0), 1e-9) && vnear(ds[2], Vector3d(5, 3, 0), 1e-9),
             "deskewing moves points by the motion left until the end of the sweep");
  }

  // ---- adaptive threshold ----
  {
    kiss_icp::AdaptiveThreshold th(2.0, 0.1, 100.0);
    QT_CHECK(th.ComputeThreshold() == 2.0, "threshold starts at its initial value");
    th.UpdateModelDeviation(Sophus::SE3d(Sophus::SO3d(), Vector3d(3, 0, 0)));
    QT_CHECK(std::fabs(th.ComputeThreshold() - std::sqrt((4.0 + 9.0) / 2)) < 1e-12, "a 3 m deviation gives sqrt((2^2 + 3^2)/2)");
    th.UpdateModelDeviation(Sophus::SE3d(Sophus::SO3d(), Vector3d(0.01, 0, 0)));
    QT_CHECK(std::fabs(th.ComputeThreshold() - std::sqrt(6.5)) < 1e-12, "a deviation below min_motion is ignored");
    th.UpdateModelDeviation(Sophus::SE3d(Sophus::SO3d::rotZ(0.01), Vector3d(0, 0, 0)));
    double rot = 2.0 * 100.0 * std::sin(0.005);   // chord at max_range for a 0.01 rad rotation
    QT_CHECK(std::fabs(th.ComputeThreshold() - std::sqrt((13.0 + rot * rot) / 3)) < 1e-9, "a rotation counts with its chord length at max range");
  }

  // ---- registration: recover a known transform ----
  {
    // Map: a lattice with 0.5 m spacing. The frame is the same lattice seen
    // from a pose displaced by less than 0.15 m everywhere, so each point's
    // nearest map point is its true partner and ICP must find the exact pose.
    Cloud l = lattice(6, 0.5);   // 2197 points within +-3 m
    kiss_icp::VoxelHashMap map(1.0, 100.0, 20);
    map.AddPoints(l);
    Eigen::Matrix<double, 6, 1> xi; xi << 0.06, -0.04, 0.03, 0.004, -0.006, 0.010;
    Sophus::SE3d truth = Sophus::SE3d::exp(xi);
    Cloud frame; frame.reserve(l.size());
    Sophus::SE3d inv = truth.inverse();
    for (const auto &p : l) frame.push_back(inv * p);
    kiss_icp::Registration reg(100, 1e-8, 2);
    Sophus::SE3d est = reg.AlignPointsToMap(frame, map, Sophus::SE3d(), 1.0, 0.5);
    double terr = (est.translation() - truth.translation()).norm();
    double rerr = (est.so3().inverse() * truth.so3()).log().norm();
    printf("  registration error: %.3g m, %.3g rad\n", terr, rerr);
    QT_CHECK(terr < 1e-6 && rerr < 1e-6, "ICP recovers the transform (0.06,-0.04,0.03 m; 0.004,-0.006,0.010 rad) to 1e-6");
    kiss_icp::VoxelHashMap empty(1.0, 100.0, 20);
    Sophus::SE3d guess(Sophus::SO3d::rotZ(0.3), Vector3d(1, 2, 3));
    QT_CHECK((reg.AlignPointsToMap(frame, empty, guess, 1.0, 0.5).matrix() - guess.matrix()).norm() == 0.0,
             "against an empty map the initial guess is returned");
  }

  // ---- the odometry pipeline on a simulated drive ----
  {
    kiss_icp::pipeline::KISSConfig cfg;
    cfg.voxel_size = 0.5; cfg.max_range = 40.0; cfg.min_range = 0.0; cfg.deskew = false; cfg.max_num_threads = 2;
    kiss_icp::pipeline::KissICP odo(cfg);
    const Cloud world = scene();
    std::vector<Eigen::Matrix4d> gt, est;
    double worst_t = 0, worst_r = 0;
    Sophus::SE3d first;
    double true_step = 0;
    const int frames = 12;
    for (int i = 0; i < frames; i++) {
      // ground truth: 0.25 m forward per frame on a path that curves sideways
      // and bobs up and down (not a straight line, not in one plane: the
      // trajectory alignment of the metric below needs that), turning 0.01 rad per frame
      Sophus::SE3d pose(Sophus::SO3d::rotZ(0.01 * i), Vector3d(-3.0 + 0.25 * i, -1.0 + 0.004 * i * i, 0.03 * (i % 4)));
      Sophus::SE3d inv = pose.inverse();
      Cloud frame; frame.reserve(world.size());
      for (const auto &p : world) frame.push_back(inv * p);
      odo.RegisterFrame(frame, {});
      // the odometry starts at identity: compare motion relative to the first frame
      if (i == 0) first = pose;
      Sophus::SE3d rel = first.inverse() * pose;
      if (i == frames - 1) true_step = (Sophus::SE3d(Eigen::Matrix4d(gt.back())).inverse() * rel).translation().norm();
      gt.push_back(rel.matrix()); est.push_back(odo.pose().matrix());
      double te = (odo.pose().translation() - rel.translation()).norm();
      double re = (odo.pose().so3().inverse() * rel.so3()).log().norm();
      if (te > worst_t) worst_t = te;
      if (re > worst_r) worst_r = re;
    }
    Sophus::SE3d last(Eigen::Matrix4d(gt.back()));
    printf("  pipeline: %zu scene points, %d frames, travelled %.2f m; worst pose error %.4f m, %.5f rad; local map %zu points\n",
           world.size(), frames, last.translation().norm(), worst_t, worst_r, odo.LocalMap().size());
    QT_CHECK(last.translation().norm() > 2.5 && vnear(odo.pose().translation(), last.translation(), 0.05), "final position within 5 cm of the truth after more than 2.5 m");
    QT_CHECK(worst_t < 0.05 && worst_r < 0.005, "every frame's pose within 5 cm and 0.005 rad");
    QT_CHECK(true_step > 0.25 && std::fabs(odo.delta().translation().norm() - true_step) < 0.02, "last frame-to-frame motion matches the true step");
    QT_CHECK(!odo.VoxelMap().Empty() && odo.LocalMap().size() > 1000, "the local map was built");
    auto [ate_rot, ate_trans] = kiss_icp::metrics::AbsoluteTrajectoryError(gt, est);
    printf("  ATE: %.3g rad, %.3g m\n", ate_rot, ate_trans);
    QT_CHECK(ate_trans < 0.05f && ate_rot < 0.01f, "absolute trajectory error below 5 cm / 0.01 rad");
    odo.Reset();
    QT_CHECK(odo.VoxelMap().Empty() && odo.pose().matrix().isIdentity(0.0), "Reset clears the map and the pose");

    // ---- metric with a known answer ----
    auto [r0, t0] = kiss_icp::metrics::AbsoluteTrajectoryError(gt, gt);
    printf("  ATE against itself: %.3g rad, %.3g m\n", r0, t0);
    QT_CHECK(r0 < 1e-6f && t0 < 1e-6f, "ATE of a trajectory against itself is 0");
    // the same path turned by 90 degrees about z with unchanged orientations:
    // alignment removes the position error and leaves a pi/2 rotation error
    std::vector<Eigen::Matrix4d> a, b;
    const double pts[5][3] = {{0, 0, 0}, {1, 0, 0}, {2, 1, 0}, {3, 1, 1}, {3, 3, 2}};
    for (const auto &p : pts) {
      Eigen::Matrix4d g = Eigen::Matrix4d::Identity(), e = Eigen::Matrix4d::Identity();
      g.block<3, 1>(0, 3) = Vector3d(p[0], p[1], p[2]);
      e.block<3, 1>(0, 3) = Vector3d(p[1], -p[0], p[2]);   // Rz(-90 deg) * p
      a.push_back(g); b.push_back(e);
    }
    auto [r90, t90] = kiss_icp::metrics::AbsoluteTrajectoryError(a, b);
    QT_CHECK(std::fabs(r90 - 1.5707963f) < 1e-4f && t90 < 1e-4f, "ATE of a path turned by 90 degrees: rotation pi/2, translation 0");
  }
  QT_END();
}
