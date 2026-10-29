// Catch2 v3: run a session over two in-binary test cases.
#include "common.h"
#include <catch2/catch_session.hpp>
#include <catch2/catch_test_macros.hpp>
TEST_CASE("arith") { REQUIRE(1 + 1 == 2); }
TEST_CASE("strings") { REQUIRE(std::string("qnx").size() == 3); }
int main() {
  QT_BEGIN("catch2");
  const char *argv[] = {"t", "--reporter", "compact"};
  int rc = Catch::Session().run(3, argv);
  QT_CHECK(rc == 0, "session passes");
  QT_END();
}
