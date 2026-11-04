/* tl-expected (header-only): tl::expected<T, E> with known results -- a
 * parser returning a value or an error, and_then chains that stop at the
 * first error, map / map_error / or_else / transform_error, value_or,
 * expected<void, E>, the bad_expected_access exception, move-only values,
 * and equality with values and unexpected errors. */
#include "common.h"
#include <tl/expected.hpp>
#include <cstdlib>
#include <memory>
#include <string>

enum class err { empty, not_a_number, negative };
static tl::expected<int, err> parse(const std::string &s) {
  if (s.empty()) return tl::make_unexpected(err::empty);
  char *end; long v = std::strtol(s.c_str(), &end, 10);
  if (*end) return tl::make_unexpected(err::not_a_number);
  return int(v);
}
static tl::expected<int, err> non_negative(int v) { if (v < 0) return tl::make_unexpected(err::negative); return v; }
static tl::expected<void, std::string> check_even(int v) { if (v % 2) return tl::make_unexpected("odd: " + std::to_string(v)); return {}; }

int main() {
  QT_BEGIN("tl-expected");
  auto a = parse("42");
  QT_CHECK(a && *a == 42 && a.value() == 42, "parse(\"42\") has value 42");
  auto b = parse("4x2");
  QT_CHECK(!b && b.error() == err::not_a_number, "parse(\"4x2\") has error not_a_number");
  QT_CHECK(parse("").error() == err::empty, "parse(\"\") has error empty");
  QT_CHECK(b.value_or(-1) == -1, "value_or(-1) on an error is -1");

  auto c = parse("-5").and_then(non_negative);
  QT_CHECK(!c && c.error() == err::negative, "and_then stops at non_negative(-5)");
  auto d = parse("7").and_then(non_negative).map([](int v) { return v * 6; });
  QT_CHECK(d && *d == 42, "parse(7) -> non_negative -> *6 = 42");
  int calls = 0;
  auto e = parse("x").and_then([&](int v) { calls++; return non_negative(v); });
  QT_CHECK(!e && e.error() == err::not_a_number && calls == 0, "and_then is not called after an error");

  auto f = parse("x").map_error([](err) { return std::string("bad input"); });
  QT_CHECK(!f && f.error() == "bad input", "map_error turns the error into a string");
  auto g = parse("").or_else([](err) -> tl::expected<int, err> { return 0; });
  QT_CHECK(g && *g == 0, "or_else recovers with 0");
  auto h = parse("9").map([](int v) { return std::to_string(v) + "!"; });
  QT_CHECK(h && *h == "9!", "map changes the value type to string");

  QT_CHECK(check_even(4).has_value(), "expected<void>: 4 is even");
  auto o = check_even(3);
  QT_CHECK(!o && o.error() == "odd: 3", "expected<void>: error \"odd: 3\"");

  bool threw = false;
  try { (void)parse("q").value(); } catch (const tl::bad_expected_access<err> &x) { threw = x.error() == err::not_a_number; }
  QT_CHECK(threw, "value() on an error throws bad_expected_access carrying the error");

  tl::expected<std::unique_ptr<int>, err> up = std::make_unique<int>(5);
  auto moved = std::move(up).map([](std::unique_ptr<int> p) { return *p + 1; });
  QT_CHECK(moved && *moved == 6, "move-only value mapped (5 + 1)");

  QT_CHECK(parse("3") == 3 && parse("3") != 4, "compares equal to a value");
  QT_CHECK(parse("") == tl::make_unexpected(err::empty), "compares equal to unexpected(empty)");
  QT_END();
}
