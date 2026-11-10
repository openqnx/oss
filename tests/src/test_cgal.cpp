// cgal: known counts from computational geometry.
//  - 2D convex hull of 36 points on a circle plus 200 inside it: the 36.
//  - Delaunay triangulation of a 10x10 grid: 100 vertices, and by Euler with
//    36 points on the hull, 2n - h - 2 = 162 triangles and 3n - h - 3 = 261 edges.
//  - 3D convex hull of a cube's corners plus interior points: 8 vertices,
//    12 triangles, a closed mesh of volume 1.
//  - Exact kernel (GMP): the diagonals of the unit square meet at exactly
//    (1/2, 1/2), and 1/3 + 1/3 + 1/3 == 1 holds exactly.
#include "common.h"
#include <cmath>
#include <vector>
#include <random>
#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/Exact_predicates_exact_constructions_kernel.h>
#include <CGAL/convex_hull_2.h>
#include <CGAL/convex_hull_3.h>
#include <CGAL/Delaunay_triangulation_2.h>
#include <CGAL/Surface_mesh.h>
#include <CGAL/Polygon_mesh_processing/measure.h>
#include <CGAL/intersections.h>
typedef CGAL::Exact_predicates_inexact_constructions_kernel K;
typedef CGAL::Exact_predicates_exact_constructions_kernel EK;

int main() {
  QT_BEGIN("cgal");
  char m[160];
  std::mt19937 rng(7);
  std::uniform_real_distribution<double> u(-0.7, 0.7);
  {
    std::vector<K::Point_2> pts, hull;
    for (int i = 0; i < 36; i++) pts.emplace_back(std::cos(i * M_PI / 18), std::sin(i * M_PI / 18));
    for (int i = 0; i < 200; i++) pts.emplace_back(u(rng), u(rng));
    std::shuffle(pts.begin(), pts.end(), rng);
    CGAL::convex_hull_2(pts.begin(), pts.end(), std::back_inserter(hull));
    snprintf(m, sizeof m, "2D hull of 236 points has %zu vertices (36)", hull.size());
    QT_CHECK(hull.size() == 36, m);
  }
  {
    CGAL::Delaunay_triangulation_2<K> dt;
    for (int x = 0; x < 10; x++) for (int y = 0; y < 10; y++) dt.insert(K::Point_2(x, y));
    snprintf(m, sizeof m, "Delaunay of a 10x10 grid: %zu vertices, %zu triangles, %zu edges (100, 162, 261)",
             dt.number_of_vertices(), dt.number_of_faces(), (size_t)std::distance(dt.finite_edges_begin(), dt.finite_edges_end()));
    QT_CHECK(dt.number_of_vertices() == 100 && dt.number_of_faces() == 162 &&
             std::distance(dt.finite_edges_begin(), dt.finite_edges_end()) == 261 && dt.is_valid(), m);
  }
  {
    std::vector<K::Point_3> pts;
    for (int i = 0; i < 8; i++) pts.emplace_back(i & 1, (i >> 1) & 1, (i >> 2) & 1);
    std::uniform_real_distribution<double> c(0.05, 0.95);
    for (int i = 0; i < 300; i++) pts.emplace_back(c(rng), c(rng), c(rng));
    CGAL::Surface_mesh<K::Point_3> sm;
    CGAL::convex_hull_3(pts.begin(), pts.end(), sm);
    double vol = CGAL::Polygon_mesh_processing::volume(sm);
    snprintf(m, sizeof m, "3D hull of a cube + 300 inside: %zu vertices, %zu faces, volume %.6f (8, 12, 1)",
             (size_t)sm.number_of_vertices(), (size_t)sm.number_of_faces(), vol);
    QT_CHECK(sm.number_of_vertices() == 8 && sm.number_of_faces() == 12 && CGAL::is_closed(sm) && std::fabs(vol - 1.0) < 1e-12, m);
  }
  {
    EK::Segment_2 a(EK::Point_2(0, 0), EK::Point_2(1, 1)), b(EK::Point_2(0, 1), EK::Point_2(1, 0));
    auto r = CGAL::intersection(a, b);
    const EK::Point_2 *p = r ? std::get_if<EK::Point_2>(&*r) : nullptr;
    QT_CHECK(p && p->x() == EK::FT(1) / 2 && p->y() == EK::FT(1) / 2, "exact: the diagonals meet at exactly (1/2, 1/2)");
    EK::FT third = EK::FT(1) / 3;
    QT_CHECK(third + third + third == EK::FT(1), "exact: 1/3 + 1/3 + 1/3 == 1");
    QT_CHECK(CGAL::orientation(EK::Point_2(0, 0), EK::Point_2(third, third), EK::Point_2(1, 1)) == CGAL::COLLINEAR,
             "exact: (0,0), (1/3,1/3), (1,1) are collinear");
  }
  QT_END();
}
