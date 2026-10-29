// OctoMap: insert points, query occupancy, round-trip through a binary stream.
#include "common.h"
#include <octomap/octomap.h>
#include <sstream>
int main() {
  QT_BEGIN("octomap");
  octomap::OcTree t(0.1);
  for (int i = 0; i < 10; i++) t.updateNode(octomap::point3d(i * 0.1f, 0, 0), true);
  auto *n = t.search(0.5, 0, 0);
  QT_CHECK(n && t.isNodeOccupied(n), "occupied cell");
  QT_CHECK(t.search(0, 5, 0) == nullptr, "unknown cell");
  std::stringstream ss; t.writeBinary(ss); octomap::OcTree u(0.1); u.readBinary(ss);
  QT_CHECK(u.getNumLeafNodes() == t.getNumLeafNodes(), "binary round trip");
  QT_END();
}
