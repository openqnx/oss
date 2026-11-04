/* nanoflann (header-only): KD-tree searches with answers known by
 * construction -- a 10x10x10 integer grid: the nearest neighbour of
 * (3.2, 4.9, 7.1) is (3, 5, 7); the 7 nearest of a grid point are itself
 * and its 6 face neighbours at distance 1; a radius-1.01 search finds those
 * 7, radius sqrt(2)+eps 19; then 2000 random points checked against brute
 * force for kNN, and the dynamic (incremental) index after adding and
 * removing points. */
#include "common.h"
#include <nanoflann.hpp>
#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <vector>

struct Cloud {
  std::vector<std::array<double, 3>> pts;
  size_t kdtree_get_point_count() const { return pts.size(); }
  double kdtree_get_pt(size_t i, size_t d) const { return pts[i][d]; }
  template <class B> bool kdtree_get_bbox(B &) const { return false; }
};
using Tree = nanoflann::KDTreeSingleIndexAdaptor<nanoflann::L2_Simple_Adaptor<double, Cloud>, Cloud, 3>;
using DynTree = nanoflann::KDTreeSingleIndexDynamicAdaptor<nanoflann::L2_Simple_Adaptor<double, Cloud>, Cloud, 3>;

int main() {
  QT_BEGIN("nanoflann");
  printf("  info nanoflann 0x%x\n", NANOFLANN_VERSION);
  Cloud g;
  for (int x = 0; x < 10; x++) for (int y = 0; y < 10; y++) for (int z = 0; z < 10; z++)
    g.pts.push_back({double(x), double(y), double(z)});
  Tree t(3, g, nanoflann::KDTreeSingleIndexAdaptorParams(10));
  t.buildIndex();

  double q[3] = {3.2, 4.9, 7.1};
  uint32_t idx; double d2;
  nanoflann::KNNResultSet<double, uint32_t> r1(1); r1.init(&idx, &d2);
  t.findNeighbors(r1, q);
  QT_CHECK(g.pts[idx][0] == 3 && g.pts[idx][1] == 5 && g.pts[idx][2] == 7, "nearest of (3.2,4.9,7.1) is (3,5,7)");
  QT_CHECK(std::fabs(d2 - (0.04 + 0.01 + 0.01)) < 1e-12, "its squared distance is 0.06");

  double c[3] = {4, 4, 4};
  uint32_t ids[7]; double ds[7];
  size_t n = t.knnSearch(c, 7, ids, ds);
  QT_CHECK(n == 7 && ds[0] == 0 && std::all_of(ds + 1, ds + 7, [](double v) { return v == 1; }),
           "7-NN of (4,4,4): itself + 6 face neighbours at distance 1");

  std::vector<nanoflann::ResultItem<uint32_t, double>> m;
  QT_CHECK(t.radiusSearch(c, 1.01 * 1.01, m) == 7, "radius 1.01 finds 7 points");
  QT_CHECK(t.radiusSearch(c, 2.0 + 1e-9, m) == 19, "radius sqrt(2) finds 19 points (1 + 6 + 12)");
  double corner[3] = {0, 0, 0};
  QT_CHECK(t.radiusSearch(corner, 1.0 + 1e-9, m) == 4, "radius 1 at the corner finds 4");

  /* random cloud vs brute force */
  Cloud r; srand(11);
  for (int i = 0; i < 2000; i++) r.pts.push_back({rand() / (double)RAND_MAX, rand() / (double)RAND_MAX, rand() / (double)RAND_MAX});
  Tree tr(3, r); tr.buildIndex();
  int bad = 0;
  for (int k = 0; k < 50; k++) {
    double p[3] = {rand() / (double)RAND_MAX, rand() / (double)RAND_MAX, rand() / (double)RAND_MAX};
    uint32_t ki[5]; double kd[5];
    tr.knnSearch(p, 5, ki, kd);
    std::vector<double> all;
    for (auto &a : r.pts) all.push_back((a[0]-p[0])*(a[0]-p[0]) + (a[1]-p[1])*(a[1]-p[1]) + (a[2]-p[2])*(a[2]-p[2]));
    std::sort(all.begin(), all.end());
    for (int j = 0; j < 5; j++) if (std::fabs(all[j] - kd[j]) > 1e-12) bad++;
  }
  QT_CHECK(bad == 0, "5-NN of 50 random queries = brute force over 2000 points");

  /* dynamic index */
  Cloud dyn; dyn.pts = {{0, 0, 0}, {10, 10, 10}};
  DynTree dt(3, dyn, nanoflann::KDTreeSingleIndexAdaptorParams(10));
  dyn.pts.push_back({5, 5, 5}); dt.addPoints(2, 2);
  double qq[3] = {6, 6, 6};
  nanoflann::KNNResultSet<double, uint32_t> r2(1); r2.init(&idx, &d2);
  dt.findNeighbors(r2, qq);
  QT_CHECK(idx == 2, "dynamic index: added point (5,5,5) is nearest to (6,6,6)");
  dt.removePoint(2);
  nanoflann::KNNResultSet<double, uint32_t> r3(1); r3.init(&idx, &d2);
  dt.findNeighbors(r3, qq);
  QT_CHECK(idx == 1, "after removing it, (10,10,10) is nearest");
  QT_END();
}
