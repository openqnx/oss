#include "common.h"
#include <flann/flann.hpp>
#include <vector>
int main() {
  QT_BEGIN("flann");
  const int n = 200, dim = 3, nq = 1, k = 4;
  std::vector<float> data(n * dim);
  for (int i = 0; i < n; ++i) { data[i*dim+0] = (float)i; data[i*dim+1] = (float)(i%10); data[i*dim+2] = 0.0f; }
  flann::Matrix<float> dataset(data.data(), n, dim);
  std::vector<float> q{50.0f, 0.0f, 0.0f};
  flann::Matrix<float> query(q.data(), nq, dim);
  std::vector<int> iv(nq * k); std::vector<float> dv(nq * k);
  flann::Matrix<int> indices(iv.data(), nq, k);
  flann::Matrix<float> dists(dv.data(), nq, k);
  flann::Index<flann::L2<float>> index(dataset, flann::KDTreeIndexParams(4));
  index.buildIndex();
  QT_CHECK(index.size() == (size_t)n, "index holds all points");
  int found = index.knnSearch(query, indices, dists, k, flann::SearchParams(32));
  QT_CHECK(found > 0, "knnSearch returned results");
  QT_CHECK(indices[0][0] >= 0 && indices[0][0] < n, "neighbour index in range");
  QT_CHECK(dists[0][0] <= dists[0][k-1], "distances sorted");
  QT_END();
}
