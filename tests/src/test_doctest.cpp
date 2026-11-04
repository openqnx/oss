/* doctest (header-only): a real doctest program -- the test runner is
 * implemented here (DOCTEST_CONFIG_IMPLEMENT) and driven from main():
 * test cases with subcases, a templated test case over three types (the
 * construct that needed ports/doctest/patches/0001 with QNX's libc++ under
 * GCC), CHECK_THROWS_AS, approximate comparison; then a deliberately
 * failing test case is run on its own (test-case filter) and must be
 * reported as exactly one failed test with two failed assertions. */
#define DOCTEST_CONFIG_IMPLEMENT
#include <doctest/doctest.h>
#include "common.h"
#include <cstdint>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

static int fact(int n) { return n <= 1 ? 1 : n * fact(n - 1); }
static int subcases_run = 0, template_runs = 0;

TEST_CASE("factorial") {
  CHECK(fact(0) == 1);
  CHECK(fact(5) == 120);
  CHECK(fact(10) == 3628800);
}

TEST_CASE("vector subcases") {
  std::vector<int> v(5);
  REQUIRE(v.size() == 5);
  SUBCASE("push_back grows") { v.push_back(1); CHECK(v.size() == 6); subcases_run++; }
  SUBCASE("clear empties") { v.clear(); CHECK(v.empty()); subcases_run++; }
}

TEST_CASE_TEMPLATE("integer limits", T, std::int8_t, std::int16_t, std::int32_t) {
  T x = static_cast<T>(-1);
  CHECK(x < 0);
  CHECK(sizeof(T) * 8 == (sizeof(T) == 1 ? 8u : sizeof(T) == 2 ? 16u : 32u));
  template_runs++;
}

TEST_CASE("exceptions and approx") {
  CHECK_THROWS_AS(throw std::runtime_error("x"), std::runtime_error);
  CHECK_NOTHROW(fact(3));
  CHECK(0.1 + 0.2 == doctest::Approx(0.3));
  CHECK(doctest::toString(42) == "42");
}

TEST_CASE("deliberately failing" * doctest::skip()) {
  CHECK(1 == 2);
  CHECK(std::string("a") == "b");
}

int main() {
  QT_BEGIN("doctest");
  printf("  info doctest %s\n", DOCTEST_VERSION_STR);
  {
    doctest::Context ctx;
    ctx.setOption("no-colors", true);
    ctx.setOption("out", "/dev/null");   /* reporters' output discarded; QT checks report */
    int rc = ctx.run();
    QT_CHECK(rc == 0, "all non-skipped test cases pass (run() = 0)");
    QT_CHECK(subcases_run == 2, "both subcases ran");
    QT_CHECK(template_runs == 3, "templated test case ran for int8, int16, int32");
  }
  {
    doctest::Context ctx;
    ctx.setOption("no-colors", true);
    ctx.setOption("out", "/dev/null");
    ctx.setOption("test-case", "deliberately failing");
    ctx.setOption("no-skip", true);
    int rc = ctx.run();
    QT_CHECK(rc != 0, "the failing test case makes run() non-zero");
  }
  {
    /* the same, through a console reporter into a stream we can read */
    std::ostringstream os;
    doctest::Context ctx;
    ctx.setOption("no-colors", true);
    ctx.setOption("test-case", "deliberately failing");
    ctx.setOption("no-skip", true);
    ctx.setCout(&os);
    ctx.run();
    std::string s = os.str();
    QT_CHECK(s.find("1 == 2") != std::string::npos, "failure report names the expression 1 == 2");
    QT_CHECK(s.find("test cases: 1 | 0 passed | 1 failed") != std::string::npos, "summary: 1 test case, 1 failed");
    QT_CHECK(s.find("assertions: 2 | 0 passed | 2 failed") != std::string::npos, "summary: 2 assertions, 2 failed");
  }
  QT_END();
}
