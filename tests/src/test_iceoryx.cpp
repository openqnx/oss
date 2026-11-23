#include "common.h"
/* iceoryx_hoofs is exercised directly: it needs no RouDi daemon, so this test
 * is self-contained. posh/RouDi is covered by the iox-roudi binary itself.
 * iceoryx 2.95 (3.0 pre-release) moved the containers from iox::cxx to iox::
 * and their headers from iceoryx_hoofs/cxx/ to iox/. */
#include <iox/vector.hpp>
#include <iox/string.hpp>
#include <iox/optional.hpp>
int main() {
  QT_BEGIN("iceoryx");
  iox::vector<int, 8> v;
  for (int i = 0; i < 8; ++i) v.push_back(i * 2);
  QT_CHECK(v.size() == 8, "fixed-capacity vector fills");
  QT_CHECK(v[7] == 14, "element value");
  QT_CHECK(!v.push_back(99), "push beyond capacity is rejected");
  iox::string<32> s("iceoryx-on-qnx");
  QT_CHECK(s.size() == 14, "fixed string size");
  QT_CHECK(s.c_str()[0] == 'i', "fixed string content");
  iox::string<4> t(iox::TruncateToCapacity, "truncated");
  QT_CHECK(t.size() == 4 && t == "trun", "string truncates to its capacity");
  iox::optional<int> o;
  QT_CHECK(!o.has_value(), "empty optional");
  o = 42;
  QT_CHECK(o.has_value() && *o == 42, "engaged optional");
  QT_END();
}
