// lanelet2: a three-lanelet road built in code, written to an OSM file in
// lat/lon (UTM projection around 49 N, 8 E) and read back, then routed.
//  - Three 10 m x 3 m lanelets in a row share their boundary points: the
//    map has 3 lanelets and 8 points; each centreline is 10 m long.
//  - After the OSM round trip every point is back within 1 mm.
//  - German vehicle traffic rules: each lanelet is passable; the routing
//    graph's shortest path from the first to the last has 3 lanelets and
//    the route is 30 m; nothing leads back from the last to the first.
#include "common.h"
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <lanelet2_core/LaneletMap.h>
#include <lanelet2_core/geometry/Lanelet.h>
#include <lanelet2_core/geometry/LineString.h>
#include <lanelet2_io/Io.h>
#include <lanelet2_projection/UTM.h>
#include <lanelet2_routing/Route.h>
#include <lanelet2_routing/RoutingGraph.h>
#include <lanelet2_traffic_rules/TrafficRulesFactory.h>
using namespace lanelet;

int main() {
  QT_BEGIN("lanelet2");
  char m[200];
  std::vector<Point3d> lp, rp;
  for (int i = 0; i < 4; i++) {
    lp.emplace_back(utils::getId(), 10.0 * i, 3.0, 0.0);
    rp.emplace_back(utils::getId(), 10.0 * i, 0.0, 0.0);
  }
  LaneletMapPtr map = std::make_shared<LaneletMap>();
  std::vector<Lanelet> lls;
  for (int i = 0; i < 3; i++) {
    LineString3d l(utils::getId(), {lp[i], lp[i + 1]}), r(utils::getId(), {rp[i], rp[i + 1]});
    l.attributes()[AttributeName::Type] = AttributeValueString::LineThin;
    l.attributes()[AttributeName::Subtype] = AttributeValueString::Solid;
    r.attributes()[AttributeName::Type] = AttributeValueString::LineThin;
    r.attributes()[AttributeName::Subtype] = AttributeValueString::Solid;
    Lanelet ll(utils::getId(), l, r);
    ll.attributes()[AttributeName::Subtype] = AttributeValueString::Road;
    ll.attributes()[AttributeName::Location] = AttributeValueString::Urban;
    ll.attributes()[AttributeName::OneWay] = true;
    map->add(ll);
    lls.push_back(ll);
  }
  snprintf(m, sizeof m, "map: %zu lanelets, %zu points (3, 8)", map->laneletLayer.size(), map->pointLayer.size());
  QT_CHECK(map->laneletLayer.size() == 3 && map->pointLayer.size() == 8, m);
  double len = geometry::length2d(lls[0]);
  snprintf(m, sizeof m, "centreline of the first lanelet: %.6f m (10)", len);
  QT_CHECK(std::fabs(len - 10.0) < 1e-9, m);

  const char *tmp = getenv("TMPDIR");
  std::string file = std::string(tmp ? tmp : "/tmp") + "/lanelet2_test_map.osm";
  projection::UtmProjector proj(Origin({49.0, 8.0}));
  ErrorMessages errs;
  write(file, *map, proj, &errs);
  QT_CHECK(errs.empty(), "write the map as OSM (lat/lon, UTM around 49 N 8 E)");
  LaneletMapPtr back = load(file, proj, &errs);
  snprintf(m, sizeof m, "read back: %zu lanelets, %zu points, %zu errors", back->laneletLayer.size(), back->pointLayer.size(), errs.size());
  QT_CHECK(back->laneletLayer.size() == 3 && back->pointLayer.size() == 8 && errs.empty(), m);
  double worst = 0;
  for (auto &p : lp) { auto q = back->pointLayer.get(p.id()); worst = std::max(worst, std::hypot(q.x() - p.x(), q.y() - p.y())); }
  for (auto &p : rp) { auto q = back->pointLayer.get(p.id()); worst = std::max(worst, std::hypot(q.x() - p.x(), q.y() - p.y())); }
  snprintf(m, sizeof m, "points round-trip within %.2e m (< 1 mm)", worst);
  QT_CHECK(worst < 1e-3, m);
  std::remove(file.c_str());

  traffic_rules::TrafficRulesPtr rules =
      traffic_rules::TrafficRulesFactory::create(Locations::Germany, Participants::Vehicle);
  QT_CHECK(rules && rules->canPass(back->laneletLayer.get(lls[0].id())), "German vehicle rules: the road lanelet is passable");
  routing::RoutingGraphUPtr graph = routing::RoutingGraph::build(*back, *rules);
  auto a = back->laneletLayer.get(lls[0].id()), c = back->laneletLayer.get(lls[2].id());
  auto path = graph->shortestPath(a, c);
  snprintf(m, sizeof m, "shortest path first -> last: %zu lanelets (3)", path ? path->size() : 0);
  QT_CHECK(path && path->size() == 3, m);
  auto route = graph->getRoute(a, c);
  double rl = route ? route->length2d() : -1;
  snprintf(m, sizeof m, "route length %.6f m (30)", rl);
  QT_CHECK(route && std::fabs(rl - 30.0) < 1e-6, m);
  QT_CHECK(!graph->shortestPath(c, a), "one-way: no path from the last lanelet back to the first");
  QT_END();
}
