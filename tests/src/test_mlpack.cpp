/* mlpack (header-only library, on Armadillo + ensmallen + cereal): machine
 * learning with answers known by construction -- k-means finds three
 * well-separated clusters (every point grouped with its own, centroids at
 * the true centres); linear regression recovers y = 5 + 3 x1 - 2 x2;
 * logistic regression (L-BFGS through ensmallen) separates two classes
 * with 100% accuracy; k-nearest-neighbours on a grid returns the expected
 * neighbours and distances; PCA finds the direction of a line; and a
 * trained model survives a cereal binary round trip. */
#include "common.h"
#include <mlpack/core.hpp>
#include <mlpack/methods/kmeans.hpp>
#include <mlpack/methods/linear_regression.hpp>
#include <mlpack/methods/logistic_regression.hpp>
#include <mlpack/methods/neighbor_search.hpp>
#include <mlpack/methods/pca.hpp>
#include <cereal/archives/binary.hpp>
#include <cmath>
#include <sstream>

using namespace mlpack;

int main() {
  QT_BEGIN("mlpack");
  printf("  info mlpack %s\n", util::GetVersion().c_str());
  arma::arma_rng::set_seed(3);
  RandomSeed(3);

  /* k-means: 3 clusters of 50 points, sigma 0.5, centres far apart */
  arma::mat centres = {{0, 10, -10}, {0, 10, 10}};
  arma::mat data(2, 150);
  for (int c = 0; c < 3; c++)
    for (int i = 0; i < 50; i++) data.col(c * 50 + i) = centres.col(c) + 0.5 * arma::randn<arma::vec>(2);
  KMeans<> km;
  arma::Row<size_t> asg; arma::mat cen;
  km.Cluster(data, 3, asg, cen);
  bool grouped = true;
  for (int c = 0; c < 3; c++) for (int i = 1; i < 50; i++) grouped &= asg(c * 50 + i) == asg(c * 50);
  QT_CHECK(grouped && asg(0) != asg(50) && asg(0) != asg(100) && asg(50) != asg(100), "k-means: each true cluster is one cluster");
  double worst = 0;
  for (int c = 0; c < 3; c++) worst = std::fmax(worst, arma::norm(cen.col(asg(c * 50)) - centres.col(c)));
  QT_CHECK(worst < 0.3, "k-means: centroids within 0.3 of the true centres");

  /* linear regression: y = 5 + 3 x1 - 2 x2 exactly */
  arma::mat X = arma::randu(2, 100) * 10;
  arma::rowvec y = 5 + 3 * X.row(0) - 2 * X.row(1);
  LinearRegression<> lr(X, y);
  arma::vec p = lr.Parameters();
  QT_CHECK(p.n_elem == 3 && std::fabs(p(0) - 5) < 1e-8 && std::fabs(p(1) - 3) < 1e-8 && std::fabs(p(2) + 2) < 1e-8,
           "linear regression: parameters [5, 3, -2]");
  arma::rowvec pred; lr.Predict(arma::mat(arma::vec({1.0, 1.0})), pred);
  QT_CHECK(std::fabs(pred(0) - 6) < 1e-8, "linear regression predicts 6 at (1, 1)");

  /* logistic regression: class 1 iff x1 + x2 > 1, with a margin */
  arma::mat L(2, 200); arma::Row<size_t> lab(200);
  for (int i = 0; i < 200; i++) {
    double a = arma::randu() * 2 - 0.5, b = arma::randu() * 2 - 0.5;
    if (std::fabs(a + b - 1) < 0.1) b += (a + b > 1 ? 0.2 : -0.2);
    L(0, i) = a; L(1, i) = b; lab(i) = a + b > 1;
  }
  LogisticRegression<> logr(L, lab);
  QT_CHECK(logr.ComputeAccuracy(L, lab) == 100.0, "logistic regression: 100% training accuracy on separable data");
  arma::Row<size_t> cls; logr.Classify(arma::mat({{0.0, 2.0}, {0.0, 2.0}}), cls);
  QT_CHECK(cls(0) == 0 && cls(1) == 1, "classifies (0,0) as 0 and (2,2) as 1");

  /* kNN on a 5x5 grid */
  arma::mat G(2, 25);
  for (int i = 0; i < 25; i++) { G(0, i) = i % 5; G(1, i) = i / 5; }
  KNN knn(G);
  arma::Mat<size_t> nb; arma::mat dist;
  knn.Search(arma::mat(arma::vec({2.1, 2.0})), 5, nb, dist);
  QT_CHECK(nb(0, 0) == 12 && std::fabs(dist(0, 0) - 0.1) < 1e-12, "kNN: nearest of (2.1, 2) is grid point 12 at 0.1");
  /* distances: 0.1 (2,2), 0.9 (3,2), sqrt(1.01) twice ((2,1), (2,3)), 1.1 (1,2) */
  QT_CHECK(std::fabs(dist(1, 0) - 0.9) < 1e-12 && std::fabs(dist(2, 0) - std::sqrt(1.01)) < 1e-12
           && std::fabs(dist(4, 0) - 1.1) < 1e-12 && nb(1, 0) == 13 && nb(4, 0) == 11, "kNN: neighbours 2-5 at 0.9, 1.005, 1.005, 1.1");

  /* PCA: points on the line y = 2x plus tiny noise */
  arma::mat P(2, 100);
  for (int i = 0; i < 100; i++) { double t = i / 10.0; P(0, i) = t; P(1, i) = 2 * t + 1e-6 * arma::randn(); }
  PCA<> pca; arma::mat tr; arma::vec ev; arma::mat evec;
  pca.Apply(P, tr, ev, evec);
  arma::vec dir = arma::normalise(arma::vec({1, 2}));
  QT_CHECK(std::fabs(std::fabs(arma::dot(evec.col(0), dir)) - 1) < 1e-9 && ev(1) / ev(0) < 1e-9,
           "PCA: first component along (1, 2), second variance ~0");

  /* model round trip through cereal */
  std::stringstream ss;
  { cereal::BinaryOutputArchive ar(ss); ar(cereal::make_nvp("lr", lr)); }
  LinearRegression<> lr2;
  { cereal::BinaryInputArchive ar(ss); ar(cereal::make_nvp("lr", lr2)); }
  QT_CHECK(arma::approx_equal(lr2.Parameters(), lr.Parameters(), "absdiff", 0), "linear model: cereal binary round trip");
  QT_END();
}
