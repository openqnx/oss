/* xtl (header-only): known answers from its tools -- base64 round trip
 * against RFC 4648 vectors, half-precision floats (exact values, overflow to
 * infinity, rounding), optional values with missing-value propagation,
 * dynamic bitsets, a fixed-capacity string, xspan, and executable_path(),
 * which needs QNX's /proc/self/exefile (ports/xtl/patches/0001). */
#include "common.h"
#include <cmath>
#include <string>
#include <vector>
#include <xtl/xbase64.hpp>
#include <xtl/xhalf_float.hpp>
#include <xtl/xoptional.hpp>
#include <xtl/xdynamic_bitset.hpp>
#include <xtl/xbasic_fixed_string.hpp>
#include <xtl/xspan.hpp>
#include <xtl/xsystem.hpp>
#include <xtl/xtl_config.hpp>

int main() {
  QT_BEGIN("xtl");
  printf("  info xtl %d.%d.%d\n", XTL_VERSION_MAJOR, XTL_VERSION_MINOR, XTL_VERSION_PATCH);
  /* RFC 4648 section 10 */
  QT_CHECK(xtl::base64encode("foobar") == "Zm9vYmFy", "base64(\"foobar\") = Zm9vYmFy");
  QT_CHECK(xtl::base64encode("fo") == "Zm8=", "base64(\"fo\") = Zm8= (padding)");
  QT_CHECK(xtl::base64decode("Zm9vYg==") == "foob", "base64 decode Zm9vYg== = foob");
  std::string bin; for (int i = 0; i < 256; i++) bin.push_back((char)i);
  QT_CHECK(xtl::base64decode(xtl::base64encode(bin)) == bin, "base64 round trip of all 256 byte values");

  xtl::half_float h(1.5f);
  QT_CHECK((float)h == 1.5f, "half 1.5 is exact");
  QT_CHECK((float)(h + xtl::half_float(2.25f)) == 3.75f, "half 1.5 + 2.25 = 3.75");
  QT_CHECK((float)xtl::half_float(65504.0f) == 65504.0f, "half max 65504");
  QT_CHECK(std::isinf((float)xtl::half_float(70000.0f)), "half 70000 overflows to inf");
  QT_CHECK((float)xtl::half_float(0.1f) == 0.0999755859375f, "half(0.1) rounds to 0.0999755859375");

  xtl::xoptional<double> a(2.0), b(3.0), m = xtl::missing<double>();
  QT_CHECK((a * b).has_value() && (a * b).value() == 6.0, "optional 2 * 3 = 6");
  QT_CHECK(!(a + m).has_value(), "optional + missing is missing");
  QT_CHECK(m.value_or(-1.0) == -1.0, "missing value_or(-1) = -1");

  xtl::xdynamic_bitset<unsigned> bs(70, false);
  bs.set(0); bs.set(33); bs.set(69);
  QT_CHECK(bs.size() == 70 && bs.count() == 3, "70-bit dynamic bitset with 3 bits set: count 3");
  QT_CHECK(bs[33] && !bs[34], "bit 33 set, bit 34 clear");
  bs.flip();
  QT_CHECK(bs.count() == 67, "flip(): 67 bits set");

  xtl::xbasic_fixed_string<char, 16> fs("qnx");
  fs += "-port";
  QT_CHECK(fs.size() == 8 && std::string(fs.c_str()) == "qnx-port" && fs.max_size() == 16, "fixed string append, max_size 16");
  QT_CHECK(fs.find("port") == 4, "fixed string find(\"port\") = 4");

  std::vector<int> v{1, 2, 3, 4, 5};
  xtl::span<int> sp(v.data() + 1, 3);
  int sum = 0; for (int x : sp) sum += x;
  QT_CHECK(sp.size() == 3 && sum == 9, "span over {2,3,4}: sum 9");

  std::string ep = xtl::executable_path();
  printf("  info executable_path = %s, prefix_path = %s\n", ep.c_str(), xtl::prefix_path().c_str());
  QT_CHECK(ep.size() > 9 && ep[0] == '/' && ep.compare(ep.size() - 9, 9, "/test_xtl") == 0,
           "executable_path() is the absolute path of this program");
  QT_END();
}
