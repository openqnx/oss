// cereal: serialise a struct to a portable binary archive and back.
#include "common.h"
#include <cereal/archives/portable_binary.hpp>
#include <cereal/types/string.hpp>
#include <cereal/types/vector.hpp>
#include <sstream>
struct P { std::string n; std::vector<int> v; template <class A> void serialize(A &a) { a(n, v); } };
int main() {
  QT_BEGIN("cereal");
  std::stringstream ss;
  { cereal::PortableBinaryOutputArchive o(ss); P p{"qnx", {8, 0}}; o(p); }
  P q; { cereal::PortableBinaryInputArchive i(ss); i(q); }
  QT_CHECK(q.n == "qnx" && q.v.size() == 2 && q.v[0] == 8, "round trip");
  QT_END();
}
