#include "common.h"
/* iceoryx_hoofs is exercised directly: it needs no RouDi daemon, so this test
 * is self-contained. posh/RouDi is covered by the iox-roudi binary itself. */
#include <iceoryx_hoofs/cxx/vector.hpp>
#include <iceoryx_hoofs/cxx/string.hpp>
#include <iceoryx_hoofs/cxx/optional.hpp>
int main() {
  QT_BEGIN("iceoryx");
  iox::cxx::vector<int, 8> v;
  for (int i = 0; i < 8; ++i) v.push_back(i * 2);
  QT_CHECK(v.size() == 8, "fixed-capacity vector fills");
  QT_CHECK(v[7] == 14, "element value");
  QT_CHECK(!v.push_back(99), "push beyond capacity is rejected");
  iox::cxx::string<32> s("iceoryx-on-qnx");
  QT_CHECK(s.size() == 14, "fixed string size");
  QT_CHECK(s.c_str()[0] == 'i', "fixed string content");
  iox::cxx::optional<int> o;
  QT_CHECK(!o.has_value(), "empty optional");
  o = 42;
  QT_CHECK(o.has_value() && *o == 42, "engaged optional");
  QT_END();
}
