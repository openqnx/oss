// fast_float (header-only): from_chars for double, float and integers with
// exact known answers -- shortest round-trip strings, the closest double to
// 0.1, halfway cases rounded to even, subnormals, overflow to infinity,
// hexadecimal-free general format, partial parses (ptr past the number),
// invalid input, and integer bases; the target's strtod is compared too
// (reported, not checked).
#include "common.h"
#include <cstdlib>
#include <cstring>
#include <cmath>
#include <limits>
#include <string>
#include <fast_float/fast_float.h>

static bool dbl(const char *s, double want) {
  double v = -1; auto r = fast_float::from_chars(s, s + strlen(s), v);
  bool ok = r.ec == std::errc() && r.ptr == s + strlen(s) && memcmp(&v, &want, sizeof v) == 0;
  if (!ok) printf("  info %s -> %.17g (wanted %.17g)\n", s, v, want);
  return ok;
}
static bool same_as_strtod(const char *s) {
  double a = -1, b = strtod(s, nullptr);
  fast_float::from_chars(s, s + strlen(s), a);
  return memcmp(&a, &b, sizeof a) == 0;
}

int main() {
  QT_BEGIN("fast-float");
  QT_CHECK(dbl("3.1415926535897931", M_PI), "pi to 17 digits is M_PI exactly");
  QT_CHECK(dbl("0.1", 0x1.999999999999ap-4), "0.1 -> 0x1.999999999999ap-4");
  QT_CHECK(dbl("9007199254740993", 9007199254740992.0), "2^53+1 rounds to even (2^53)");
  QT_CHECK(dbl("9007199254740995", 9007199254740996.0), "2^53+3 rounds to even (2^53+4)");
  QT_CHECK(dbl("4.9406564584124654e-324", std::numeric_limits<double>::denorm_min()), "smallest subnormal");
  QT_CHECK(dbl("2.2250738585072011e-308", 0x0.fffffffffffffp-1022), "largest subnormal");
  QT_CHECK(dbl("1.7976931348623157e308", std::numeric_limits<double>::max()), "DBL_MAX");
  QT_CHECK(dbl("-0", -0.0) && std::signbit(-0.0), "-0 keeps its sign");
  {
    double v; const char *s = "1e400";
    auto r = fast_float::from_chars(s, s + 5, v);
    QT_CHECK(r.ptr == s + 5 && std::isinf(v), "1e400 overflows to infinity");
  }
  {
    float f; const char *s = "16777217";
    fast_float::from_chars(s, s + strlen(s), f);
    QT_CHECK(f == 16777216.0f, "float: 2^24+1 rounds to 2^24");
    s = "3.4028235e38"; fast_float::from_chars(s, s + strlen(s), f);
    QT_CHECK(f == std::numeric_limits<float>::max(), "float: FLT_MAX");
  }
  {
    double v; const char *s = "12.5e3xyz";
    auto r = fast_float::from_chars(s, s + strlen(s), v);
    QT_CHECK(r.ec == std::errc() && v == 12500.0 && r.ptr == s + 6, "partial parse stops after 12.5e3");
    s = "abc";
    r = fast_float::from_chars(s, s + 3, v);
    QT_CHECK(r.ec == std::errc::invalid_argument && r.ptr == s, "invalid input: invalid_argument");
    s = "1.5e2";
    r = fast_float::from_chars(s, s + strlen(s), v, fast_float::chars_format::fixed);
    QT_CHECK(r.ec == std::errc() && v == 1.5 && r.ptr == s + 3, "chars_format::fixed ignores the exponent");
  }
  {
    int i = 0; const char *s = "-2147483648";
    auto r = fast_float::from_chars(s, s + strlen(s), i);
    QT_CHECK(r.ec == std::errc() && i == INT32_MIN, "int: INT32_MIN");
    s = "2147483648"; r = fast_float::from_chars(s, s + strlen(s), i);
    QT_CHECK(r.ec == std::errc::result_out_of_range, "int: 2^31 out of range");
    uint64_t u = 0; s = "ff"; r = fast_float::from_chars(s, s + 2, u, 16);
    QT_CHECK(r.ec == std::errc() && u == 255, "base 16: ff = 255");
    s = "777"; r = fast_float::from_chars(s, s + 3, u, 8);
    QT_CHECK(r.ec == std::errc() && u == 511, "base 8: 777 = 511");
  }
  const char *mix[] = { "1e23", "8.98846567431158e307", "2.2250738585072012e-308", "123456789012345678901234567890",
                        "0.000000000000000000000000000001", "7.2057594037927933e16", "1.00000005960464477539062499" };
  int agree = 0;
  for (const char *s : mix) agree += same_as_strtod(s);
  // informational: a disagreement would be the C library's rounding, not fast_float's
  printf("  info %d of 7 hard cases agree bit for bit with the target's strtod\n", agree);
  QT_CHECK(dbl("1e23", 1e23) && dbl("7.2057594037927933e16", 72057594037927936.0), "1e23 and 2^56 as the compiler rounds them");
  QT_END();
}
