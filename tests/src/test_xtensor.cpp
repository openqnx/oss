/* xtensor (header-only, built with XTENSOR_USE_XSIMD): NumPy-style arrays
 * with known answers -- broadcasting a column against a row, reductions
 * over an axis, views and strided slices, reshape and transpose, linspace,
 * a lazy expression evaluated only on assignment, sort/argmax, adapting an
 * existing buffer, masked assignment, a cumulative sum, the npy file
 * format written and read back, and printing. Element-wise maths on
 * contiguous float arrays goes through xsimd. */
#include "common.h"
#include <xtensor/containers/xarray.hpp>
#include <xtensor/containers/xtensor.hpp>
#include <xtensor/containers/xadapt.hpp>
#include <xtensor/core/xmath.hpp>
#include <xtensor/core/xnoalias.hpp>
#include <xtensor/generators/xbuilder.hpp>
#include <xtensor/views/xview.hpp>
#include <xtensor/views/xmasked_view.hpp>
#include <xtensor/misc/xsort.hpp>
#include <xtensor/misc/xmanipulation.hpp>
#include <xtensor/io/xio.hpp>
#include <xtensor/io/xnpy.hpp>
#include <xtensor/reducers/xaccumulator.hpp>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <sstream>
#include <vector>

int main() {
  QT_BEGIN("xtensor");
  printf("  info xtensor %d.%d.%d, xsimd %s\n", XTENSOR_VERSION_MAJOR, XTENSOR_VERSION_MINOR, XTENSOR_VERSION_PATCH,
#ifdef XTENSOR_USE_XSIMD
         "on"
#else
         "off"
#endif
  );
#ifdef XTENSOR_USE_XSIMD
  QT_CHECK(1, "built with XTENSOR_USE_XSIMD");
#else
  QT_CHECK(0, "built with XTENSOR_USE_XSIMD");
#endif
  xt::xarray<double> col = {1., 2., 3.};
  col.reshape({3, 1});                            /* 3x1 */
  xt::xarray<double> row = {10., 20., 30., 40.};     /* 4   */
  xt::xarray<double> m = col * row;              /* 3x4 */
  QT_CHECK(m.shape()[0] == 3 && m.shape()[1] == 4, "broadcast 3x1 * 4 -> 3x4");
  QT_CHECK(m(2, 3) == 120 && m(0, 0) == 10, "m(2,3) = 120, m(0,0) = 10");
  xt::xarray<double> s0 = xt::sum(m, {0}), s1 = xt::sum(m, {1});
  QT_CHECK(s0 == xt::xarray<double>({60., 120., 180., 240.}), "sum over axis 0 = [60 120 180 240]");
  QT_CHECK(s1 == xt::xarray<double>({100., 200., 300.}), "sum over axis 1 = [100 200 300]");
  QT_CHECK(xt::sum(m)() == 600 && xt::mean(m)() == 50, "total 600, mean 50");

  auto v = xt::view(m, xt::range(1, 3), xt::range(0, 4, 2));    /* rows 1-2, cols 0,2 */
  QT_CHECK(v.shape()[0] == 2 && v.shape()[1] == 2 && v(0, 0) == 20 && v(1, 1) == 90, "view rows 1:3, cols ::2");
  xt::xarray<int> ar = xt::arange<int>(12);
  ar.reshape({3, 4});
  auto tr = xt::transpose(ar);
  QT_CHECK(tr.shape()[0] == 4 && tr(3, 2) == 11 && tr(1, 0) == 1, "arange(12).reshape(3,4).T");
  xt::xtensor<double, 1> ls = xt::linspace<double>(0.0, 1.0, 5);
  QT_CHECK(ls(1) == 0.25 && ls(4) == 1.0, "linspace(0,1,5)");

  /* lazy: nothing is computed until assignment */
  xt::xtensor<float, 1> a = xt::arange<float>(1000), b = xt::ones<float>({1000});
  auto expr = xt::sqrt(a * a) + 2.0f * b;
  xt::xtensor<float, 1> c = expr;
  QT_CHECK(c(0) == 2.0f && c(999) == 1001.0f, "lazy sqrt(a*a) + 2b evaluated on assignment");
  xt::noalias(c) = xt::exp(xt::zeros<float>({1000}));
  QT_CHECK(xt::all(xt::equal(c, 1.0f)), "noalias exp(0) = 1 everywhere");

  xt::xarray<int> u = {5, 1, 4, 2, 3};
  QT_CHECK(xt::sort(u) == xt::xarray<int>({1, 2, 3, 4, 5}), "sort");
  QT_CHECK(xt::argmax(u)() == 0 && xt::argmin(u)() == 1, "argmax 0, argmin 1");
  QT_CHECK(xt::cumsum(u) == xt::xarray<int>({5, 6, 10, 12, 15}), "cumsum");

  std::vector<double> buf = {1, 2, 3, 4, 5, 6};
  auto ad = xt::adapt(buf, {2, 3});
  ad(1, 2) = 60;
  QT_CHECK(buf[5] == 60 && ad(0, 1) == 2, "adapt writes through to the std::vector");
  xt::xarray<double> mk = {1., -2., 3., -4.};
  xt::masked_view(mk, mk < 0.0) = 0;
  QT_CHECK(mk == xt::xarray<double>({1., 0., 3., 0.}), "masked assignment of negatives");
  QT_CHECK(xt::allclose(xt::flip(ls, 0), xt::xarray<double>({1., 0.75, 0.5, 0.25, 0.})), "flip");

  const char *td = std::getenv("TMPDIR");
  std::string path = std::string(td && *td ? td : "/tmp") + "/xtensor_test.npy";
  xt::dump_npy(path, m);
  auto back = xt::load_npy<double>(path);
  QT_CHECK(back == m, "npy file written and read back equal");
  std::remove(path.c_str());

  std::ostringstream os; os << xt::xarray<int>({{1, 2}, {3, 4}});
  QT_CHECK(os.str() == "{{1, 2},\n {3, 4}}", "printing {{1, 2},\\n {3, 4}}");
  QT_END();
}
