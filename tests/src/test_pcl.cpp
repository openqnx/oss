#include "common.h"
#include <pcl/point_cloud.h>
#include <pcl/point_types.h>
#include <pcl/filters/voxel_grid.h>
#include <pcl/kdtree/kdtree_flann.h>
#include <pcl/common/common.h>
int main() {
  QT_BEGIN("pcl");
  pcl::PointCloud<pcl::PointXYZ>::Ptr cloud(new pcl::PointCloud<pcl::PointXYZ>);
  for (int i = 0; i < 1000; ++i)
    cloud->push_back(pcl::PointXYZ(i * 0.01f, (i % 10) * 0.1f, (i % 7) * 0.2f));
  QT_CHECK(cloud->size() == 1000, "cloud populated");
  pcl::PointXYZ mn, mx;
  pcl::getMinMax3D(*cloud, mn, mx);
  QT_CHECK(mx.x > mn.x, "getMinMax3D");
  pcl::PointCloud<pcl::PointXYZ>::Ptr filtered(new pcl::PointCloud<pcl::PointXYZ>);
  pcl::VoxelGrid<pcl::PointXYZ> vg;
  vg.setInputCloud(cloud); vg.setLeafSize(0.5f, 0.5f, 0.5f);
  vg.filter(*filtered);
  QT_CHECK(filtered->size() > 0 && filtered->size() < cloud->size(), "voxel grid downsampled");
  pcl::KdTreeFLANN<pcl::PointXYZ> kd;
  kd.setInputCloud(cloud);
  std::vector<int> idx(5); std::vector<float> d2(5);
  int found = kd.nearestKSearch(pcl::PointXYZ(5.0f, 0.5f, 0.4f), 5, idx, d2);
  QT_CHECK(found == 5, "kd-tree knn search returned 5 neighbours");
  QT_CHECK(d2[0] <= d2[4], "neighbours sorted by distance");
  QT_END();
}
