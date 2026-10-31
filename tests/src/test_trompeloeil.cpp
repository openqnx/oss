// trompeloeil (header-only mocking): expectations with argument matchers,
// return values computed from arguments, call counts, sequences, side
// effects -- and the violations: an unexpected call (fatal, reported through
// a custom reporter that throws) and an unfulfilled expectation.
#include "common.h"
#include <trompeloeil.hpp>
#include <stdexcept>
#include <string>

struct Store {
  virtual ~Store() = default;
  virtual int get(int key) = 0;
  virtual void put(const std::string &v) = 0;
  virtual bool ready() const = 0;
};
struct MockStore : Store {
  MAKE_MOCK1(get, int(int), override);
  MAKE_MOCK1(put, void(const std::string &), override);
  MAKE_CONST_MOCK0(ready, bool(), override);
};

static int violations = 0;
static std::string last;

int main() {
  QT_BEGIN("trompeloeil");
  trompeloeil::set_reporter([](trompeloeil::severity s, const char *, unsigned long, const std::string &msg) {
    violations++; last = msg;
    if (s == trompeloeil::severity::fatal) throw std::runtime_error(msg);
  });
  MockStore m;
  {
    REQUIRE_CALL(m, get(3)).RETURN(_1 * 2).TIMES(2);
    int a = m.get(3), b = m.get(3);
    QT_CHECK(a == 6 && b == 6, "REQUIRE_CALL get(3) returns _1*2, twice");
  }
  QT_CHECK(violations == 0, "expectation met: no violation");
  {
    trompeloeil::sequence seq;
    std::string log;
    REQUIRE_CALL(m, put("a")).IN_SEQUENCE(seq).LR_SIDE_EFFECT(log += _1);
    REQUIRE_CALL(m, put("b")).IN_SEQUENCE(seq).LR_SIDE_EFFECT(log += _1);
    ALLOW_CALL(m, ready()).RETURN(true);
    m.put("a"); m.put("b");
    QT_CHECK(log == "ab" && m.ready(), "sequence a, b with side effects; ALLOW_CALL");
  }
  QT_CHECK(violations == 0, "sequence respected: no violation");
  {
    ALLOW_CALL(m, get(trompeloeil::gt(10))).RETURN(42);
    FORBID_CALL(m, get(0));
    QT_CHECK(m.get(11) == 42 && m.get(100) == 42, "matcher gt(10)");
    bool threw = false;
    try { m.get(0); } catch (const std::runtime_error &) { threw = true; }
    QT_CHECK(threw && violations == 1 && last.find("Match of forbidden call") != std::string::npos, "FORBID_CALL reported (fatal)");
  }
  {
    bool threw = false;
    try { m.get(5); } catch (const std::runtime_error &) { threw = true; }
    QT_CHECK(threw && violations == 2 && last.find("No match for call") != std::string::npos, "unexpected call get(5) reported");
  }
  {
    REQUIRE_CALL(m, get(1)).RETURN(1);
  }
  QT_CHECK(violations == 3 && last.find("Unfulfilled expectation") != std::string::npos, "unfulfilled expectation reported");
  if (violations != 3) printf("  info last report: %s\n", last.c_str());
  QT_END();
}
