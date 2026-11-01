// Cartographer: a tiny 2D SLAM run with a known answer. A simulated robot
// drives 1.5 m along a gentle curve through a 8 m x 6 m room with two
// pillars; 30 laser scans (180 rays each, ray-cast against the room) and a
// drifting wheel odometry (10 % short, turns 20 % too large: 15 cm and 3.5
// deg off at the end) go through MapBuilder with Cartographer's own Lua
// configuration files, and every local SLAM pose has to match the true pose
// (relative to the start) to within one grid cell (5 cm) and 1 deg. The
// robot stands still for the first 10 scans, as a real one does while its
// first submap fills: a robot that starts at full speed without odometry
// is not tracked by Cartographer's local SLAM (scan matching against a
// one-scan submap does not overcome the zero-velocity prediction), on
// Linux exactly as on QNX. Also checked: the Lua parameter dictionary,
// rigid-transform algebra, a submap built, the pose graph's node poses after
// the final optimisation, a probability grid's cells and a scan inserted
// into one.
#include "common.h"
#include "cartographer/common/configuration_file_resolver.h"
#include "cartographer/common/lua_parameter_dictionary.h"
#include "cartographer/common/time.h"
#include "cartographer/sensor/odometry_data.h"
#include "cartographer/mapping/2d/probability_grid.h"
#include "cartographer/mapping/2d/probability_grid_range_data_inserter_2d.h"
#include "cartographer/mapping/map_builder.h"
#include "cartographer/mapping/value_conversion_tables.h"
#include "cartographer/transform/rigid_transform.h"
#include "cartographer/transform/transform.h"
#include <cmath>
#include <cstdlib>
#include <mutex>
#include <vector>

using namespace cartographer;

namespace {
struct Seg { double x0, y0, x1, y1; };
std::vector<Seg> Room() {
  std::vector<Seg> s = {{-3, -2, 5, -2}, {5, -2, 5, 4}, {5, 4, -3, 4}, {-3, 4, -3, -2}};
  auto box = [&](double cx, double cy, double h) {
    s.push_back({cx - h, cy - h, cx + h, cy - h}); s.push_back({cx + h, cy - h, cx + h, cy + h});
    s.push_back({cx + h, cy + h, cx - h, cy + h}); s.push_back({cx - h, cy + h, cx - h, cy - h});
  };
  box(2.0, 1.5, 0.3);    // pillars: break the room's symmetry
  box(-1.5, -1.0, 0.2);
  return s;
}
// Distance along the ray from (px,py) in direction a to the nearest wall.
double Cast(const std::vector<Seg> &room, double px, double py, double a) {
  double dx = std::cos(a), dy = std::sin(a), best = 1e9;
  for (const Seg &s : room) {
    double ex = s.x1 - s.x0, ey = s.y1 - s.y0, den = dx * ey - dy * ex;
    if (std::fabs(den) < 1e-12) continue;
    double t = ((s.x0 - px) * ey - (s.y0 - py) * ex) / den;
    double u = ((s.x0 - px) * dy - (s.y0 - py) * dx) / den;
    if (t > 0 && u >= 0 && u <= 1 && t < best) best = t;
  }
  return best;
}
// Ground truth: start at (0,0,0), 5 cm and 0.6 deg per scan. Odometry: the
// same motion as a drifting wheel odometry reports it -- distances 10 %
// short, turns 20 % too large.
const int kStill = 10;   // scans before the robot starts moving
transform::Rigid2d Path(int i, double scale, double turn) {
  i = std::max(0, i - kStill);
  double step = turn * 0.6 * M_PI / 180, th = i * step, x = 0, y = 0;
  for (int k = 0; k < i; ++k) {
    double a = (k + 0.5) * step;
    x += scale * 0.05 * std::cos(a); y += scale * 0.05 * std::sin(a);
  }
  return transform::Rigid2d({x, y}, th);
}
transform::Rigid2d Truth(int i) { return Path(i, 1.0, 1.0); }
transform::Rigid2d Odometry(int i) { return Path(i, 0.9, 1.2); }
}  // namespace

int main() {
  QT_BEGIN("cartographer");
  const char *env = getenv("CARTOGRAPHER_CONFIG_DIR");
  std::string cfgdir = env ? env : "/usr/local/share/cartographer/configuration_files";
  printf("  info configuration files: %s\n", cfgdir.c_str());

  // ---- transforms ----
  transform::Rigid2d a({1, 2}, M_PI / 2), b({2, 0}, 0);
  transform::Rigid2d c = a * b;
  QT_CHECK(std::fabs(c.translation().x() - 1) < 1e-12 && std::fabs(c.translation().y() - 4) < 1e-12 &&
           std::fabs(c.normalized_angle() - M_PI / 2) < 1e-12, "Rigid2d composition");
  transform::Rigid3d r3 = transform::Embed3D(a);
  QT_CHECK(std::fabs(transform::GetYaw(r3) - M_PI / 2) < 1e-12 &&
           (r3.inverse() * r3).translation().norm() < 1e-12, "Embed3D, GetYaw, inverse");

  // ---- Lua configuration ----
  {
    auto d = common::LuaParameterDictionary::NonReferenceCounted(
        "include \"map_builder.lua\"\nreturn { n = 6 * 7, s = \"q\" .. \"nx\", t = { f = math.rad(180.) },"
        " threads = MAP_BUILDER.num_background_threads }",
        absl::make_unique<common::ConfigurationFileResolver>(std::vector<std::string>{cfgdir}));
    QT_CHECK(d->GetInt("n") == 42 && d->GetString("s") == "qnx" &&
             std::fabs(d->GetDictionary("t")->GetDouble("f") - M_PI) < 1e-12,
             "Lua dictionary: arithmetic, strings, nested tables");
    QT_CHECK(d->GetInt("threads") > 0, "Lua include of the installed map_builder.lua");
  }

  // ---- probability grid ----
  {
    mapping::ValueConversionTables tables;
    mapping::ProbabilityGrid grid(mapping::MapLimits(0.05, {1.0, 1.0}, mapping::CellLimits(40, 40)), &tables);
    Eigen::Array2i cell = grid.limits().GetCellIndex(Eigen::Vector2f(0.27f, -0.33f));
    grid.SetProbability(cell, 0.75f);
    QT_CHECK(std::fabs(grid.GetProbability(cell) - 0.75f) < 1e-3 &&
             std::fabs(grid.GetProbability(cell + Eigen::Array2i(1, 0)) - mapping::kMinProbability) < 1e-6,
             "probability grid stores a cell; unknown cells read the minimum");
    Eigen::Vector2f centre = grid.limits().GetCellCenter(cell);
    QT_CHECK(std::fabs(centre.x() - 0.275f) < 0.026f && std::fabs(centre.y() + 0.325f) < 0.026f,
             "cell centre is within half a cell of the point");
  }

  // ---- one scan of the room inserted into a probability grid ----
  {
    const std::vector<Seg> room = Room();
    auto d = common::LuaParameterDictionary::NonReferenceCounted(
        "return { insert_free_space = true, hit_probability = 0.55, miss_probability = 0.49 }",
        absl::make_unique<common::ConfigurationFileResolver>(std::vector<std::string>{cfgdir}));
    mapping::ProbabilityGridRangeDataInserter2D inserter(mapping::CreateProbabilityGridRangeDataInserterOptions2D(d.get()));
    mapping::ValueConversionTables tables;
    mapping::ProbabilityGrid grid(mapping::MapLimits(0.05, {8.0, 8.0}, mapping::CellLimits(320, 320)), &tables);
    std::vector<sensor::RangefinderPoint> hits;
    for (int k = 0; k < 360; ++k) {
      double a = -M_PI + 2 * M_PI * k / 360, r = Cast(room, 0, 0, a);
      hits.push_back({Eigen::Vector3f(r * std::cos(a), r * std::sin(a), 0.f)});
    }
    inserter.Insert(sensor::RangeData{Eigen::Vector3f::Zero(), sensor::PointCloud(hits), {}}, &grid);
    grid.FinishUpdate();
    auto P = [&](float x, float y) { return grid.GetProbability(grid.limits().GetCellIndex(Eigen::Vector2f(x, y))); };
    auto K = [&](float x, float y) { return grid.IsKnown(grid.limits().GetCellIndex(Eigen::Vector2f(x, y))); };
    printf("  info inserted scan: p(wall 5,0) = %.3f, p(free 2.5,0) = %.3f, known(7,0) = %d\n", P(5.0f, 0.f), P(2.5f, 0.f), K(7.f, 0.f));
    QT_CHECK(std::fabs(P(5.0f, 0.f) - 0.55f) < 0.01f && std::fabs(P(-3.0f, 0.f) - 0.55f) < 0.01f,
             "range data inserter: wall cells hit (p = hit_probability 0.55)");
    QT_CHECK(std::fabs(P(2.5f, 0.f) - 0.49f) < 0.01f && std::fabs(P(0.f, 3.f) - 0.49f) < 0.01f,
             "range data inserter: cells along the rays free (p = miss_probability 0.49)");
    QT_CHECK(!K(7.f, 0.f), "range data inserter: cells behind the wall stay unknown");
  }

  // ---- 2D SLAM ----
  // CARTOGRAPHER_TEST_LUA: extra Lua lines for experiments (none by default).
  const char *extra_lua = getenv("CARTOGRAPHER_TEST_LUA");
  auto dict = common::LuaParameterDictionary::NonReferenceCounted(std::string(R"(
include "map_builder.lua"
include "trajectory_builder.lua"
MAP_BUILDER.use_trajectory_builder_2d = true
MAP_BUILDER.num_background_threads = 2
TRAJECTORY_BUILDER_2D.use_imu_data = false
TRAJECTORY_BUILDER_2D.min_range = 0.1
TRAJECTORY_BUILDER_2D.max_range = 12.
TRAJECTORY_BUILDER_2D.missing_data_ray_length = 12.
TRAJECTORY_BUILDER_2D.use_online_correlative_scan_matching = true
TRAJECTORY_BUILDER_2D.motion_filter.max_distance_meters = 0.1
TRAJECTORY_BUILDER_2D.submaps.num_range_data = 20
POSE_GRAPH.optimize_every_n_nodes = 10
)") + (extra_lua ? extra_lua : "") + R"(
return { map_builder = MAP_BUILDER, trajectory_builder = TRAJECTORY_BUILDER }
)", absl::make_unique<common::ConfigurationFileResolver>(std::vector<std::string>{cfgdir}));
  auto mb_options = mapping::CreateMapBuilderOptions(dict->GetDictionary("map_builder").get());
  auto tb_options = mapping::CreateTrajectoryBuilderOptions(dict->GetDictionary("trajectory_builder").get());
  auto builder = mapping::CreateMapBuilder(mb_options);

  std::mutex mu;
  std::vector<std::pair<common::Time, transform::Rigid3d>> local;
  int inserted = 0;
  using SensorId = mapping::TrajectoryBuilderInterface::SensorId;
  const SensorId scan_id{SensorId::SensorType::RANGE, "scan"};
  const SensorId odom_id{SensorId::SensorType::ODOMETRY, "odom"};
  int tid = builder->AddTrajectoryBuilder(
      {scan_id, odom_id}, tb_options,
      [&](int, common::Time time, transform::Rigid3d pose, sensor::RangeData,
          std::unique_ptr<const mapping::TrajectoryBuilderInterface::InsertionResult> ins) {
        std::lock_guard<std::mutex> l(mu);
        local.emplace_back(time, pose);
        if (ins) ++inserted;
      });
  mapping::TrajectoryBuilderInterface *tb = builder->GetTrajectoryBuilder(tid);
  QT_CHECK(tb != nullptr, "2D trajectory builder created");

  const std::vector<Seg> room = Room();
  const int kScans = kStill + 30, kRays = 180;
  const common::Time t0 = common::FromUniversal(638000000000000000LL);  // scan i at t0 + 0.1 s * i
  for (int i = 0; tb && i < kScans; ++i) {
    transform::Rigid2d T = Truth(i);
    sensor::TimedPointCloud pts;
    for (int k = 0; k < kRays; ++k) {
      double ang = -M_PI + 2 * M_PI * k / kRays;   // in the sensor frame
      double d = Cast(room, T.translation().x(), T.translation().y(), T.normalized_angle() + ang);
      if (d > 11) continue;
      pts.push_back({Eigen::Vector3f(d * std::cos(ang), d * std::sin(ang), 0.f), 0.f});
    }
    tb->AddSensorData(odom_id.id, sensor::OdometryData{t0 + common::FromSeconds(0.1 * i), transform::Embed3D(Odometry(i))});
    tb->AddSensorData(scan_id.id, sensor::TimedPointCloudData{
        t0 + common::FromSeconds(0.1 * i), Eigen::Vector3f::Zero(), pts, {}});
  }
  builder->FinishTrajectory(tid);
  builder->pose_graph()->RunFinalOptimization();

  double worst_t = 0, worst_r = 0;
  {
    std::lock_guard<std::mutex> l(mu);
    QT_CHECK((int)local.size() >= kScans - 1, "a local SLAM result for every scan");
    for (size_t j = 0; j < local.size(); ++j) {
      int i = (int)std::lround(common::ToSeconds(local[j].first - t0) / 0.1);
      transform::Rigid2d est = transform::Project2D(local[j].second), tr = Truth(i);
      if (getenv("CARTOGRAPHER_TEST_VERBOSE"))
        printf("  info scan %2d: est (%.3f, %.3f, %.2f deg) true (%.3f, %.3f, %.2f deg)\n", i, est.translation().x(),
               est.translation().y(), est.normalized_angle() * 180 / M_PI, tr.translation().x(), tr.translation().y(),
               tr.normalized_angle() * 180 / M_PI);
      worst_t = std::max(worst_t, (est.translation() - tr.translation()).norm());
      worst_r = std::max(worst_r, std::fabs(common::NormalizeAngleDifference(est.normalized_angle() - tr.normalized_angle())));
    }
    printf("  info %zu scans, %d inserted; worst error %.4f m, %.3f deg; last pose (%.3f, %.3f, %.2f deg)\n",
           local.size(), inserted, worst_t, worst_r * 180 / M_PI,
           local.empty() ? 0. : local.back().second.translation().x(), local.empty() ? 0. : local.back().second.translation().y(),
           local.empty() ? 0. : transform::GetYaw(local.back().second) * 180 / M_PI);
  }
  {
    transform::Rigid2d o = Odometry(kScans - 1), tr = Truth(kScans - 1);
    printf("  info odometry alone ends %.3f m, %.2f deg from the true pose\n", (o.translation() - tr.translation()).norm(),
           std::fabs(o.normalized_angle() - tr.normalized_angle()) * 180 / M_PI);
  }
  // One cell of the submaps' 5 cm grid: what local SLAM resolves (a Linux
  // build of the same sources gives the same 3.2 cm / 0.29 deg).
  QT_CHECK(worst_t < 0.05 && worst_r < M_PI / 180, "local SLAM corrects the odometry onto the true trajectory (5 cm, 1 deg)");
  QT_CHECK(inserted >= 10, "the motion filter inserts a node every 10 cm");

  auto nodes = builder->pose_graph()->GetTrajectoryNodePoses();
  auto submaps = builder->pose_graph()->GetAllSubmapData();
  printf("  info pose graph: %zu nodes, %zu submaps\n", (size_t)nodes.size(), (size_t)submaps.size());
  QT_CHECK(nodes.size() >= 10 && submaps.size() >= 1, "pose graph holds the nodes and a submap");
  double worst_g = 0, worst_gr = 0;
  int checked = 0;
  for (const auto &n : nodes) {
    if (!n.data.constant_pose_data.has_value()) continue;
    int i = (int)std::lround(common::ToSeconds(n.data.constant_pose_data->time - t0) / 0.1);
    transform::Rigid2d g = transform::Project2D(n.data.global_pose), tr = Truth(i);
    worst_g = std::max(worst_g, (g.translation() - tr.translation()).norm());
    worst_gr = std::max(worst_gr, std::fabs(common::NormalizeAngleDifference(g.normalized_angle() - tr.normalized_angle())));
    ++checked;
  }
  printf("  info optimised global poses vs truth: worst %.4f m, %.3f deg (%d nodes)\n", worst_g, worst_gr * 180 / M_PI, checked);
  // The pose graph also weighs the (drifting) odometry, heavily by default,
  // so its poses sit between scan matching and odometry: at most half the
  // odometry's own error (15 cm, 3.5 deg at the end). Linux: 4.6 cm, 0.82 deg.
  QT_CHECK(checked >= 10 && worst_g < 0.075 && worst_gr < 1.75 * M_PI / 180,
           "optimised global node poses: within half the odometry's error of the truth");
  QT_END();
}
