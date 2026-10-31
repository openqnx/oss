/* FXdiv (header-only): division by a runtime-invariant divisor through a
 * precomputed multiplier, checked against the hardware divide for many
 * divisors and dividends (32-bit, 64-bit, size_t), with the edge cases
 * 1, 2^k and the largest values. */
#include "common.h"
#include <fxdiv.h>
#include <stdint.h>

static uint64_t s = 0x9E3779B97F4A7C15ull;
static uint64_t rnd(void) { s ^= s << 13; s ^= s >> 7; s ^= s << 17; return s; }

int main(void) {
  QT_BEGIN("fxdiv");
  struct fxdiv_divisor_uint32_t d7 = fxdiv_init_uint32_t(7);
  QT_CHECK(fxdiv_quotient_uint32_t(100, d7) == 14 && fxdiv_remainder_uint32_t(100, d7) == 2, "100 / 7 = 14 r 2");
  struct fxdiv_result_uint32_t r = fxdiv_divide_uint32_t(UINT32_MAX, d7);
  QT_CHECK(r.quotient == UINT32_MAX / 7 && r.remainder == UINT32_MAX % 7, "UINT32_MAX / 7");
  QT_CHECK(fxdiv_round_down_uint32_t(100, d7) == 98, "round 100 down to a multiple of 7 = 98");

  long bad32 = 0, bad64 = 0, badsz = 0, n = 0;
  uint32_t ds32[] = { 1, 2, 3, 5, 7, 10, 64, 641, 65535, 65536, 1000003, 0x7FFFFFFF, 0x80000000u, UINT32_MAX };
  for (size_t i = 0; i < sizeof ds32 / sizeof ds32[0] + 2000; i++) {
    uint32_t d = i < sizeof ds32 / sizeof ds32[0] ? ds32[i] : (uint32_t)(rnd() >> (32 + rnd() % 31));
    if (!d) d = 1;
    struct fxdiv_divisor_uint32_t fd = fxdiv_init_uint32_t(d);
    struct fxdiv_divisor_uint64_t fd64 = fxdiv_init_uint64_t(((uint64_t)d << 13) | 1);
    struct fxdiv_divisor_size_t fdz = fxdiv_init_size_t((size_t)d);
    for (int j = 0; j < 200; j++, n++) {
      uint32_t a = j == 0 ? UINT32_MAX : j == 1 ? 0 : (uint32_t)rnd();
      struct fxdiv_result_uint32_t q = fxdiv_divide_uint32_t(a, fd);
      if (q.quotient != a / d || q.remainder != a % d) bad32++;
      uint64_t a64 = j == 0 ? UINT64_MAX : rnd(), d64 = ((uint64_t)d << 13) | 1;
      struct fxdiv_result_uint64_t q64 = fxdiv_divide_uint64_t(a64, fd64);
      if (q64.quotient != a64 / d64 || q64.remainder != a64 % d64) bad64++;
      size_t az = (size_t)rnd();
      if (fxdiv_quotient_size_t(az, fdz) != az / d) badsz++;
    }
  }
  char what[128];
  snprintf(what, sizeof what, "uint32: %ld divisions match the hardware divide", n);
  QT_CHECK(bad32 == 0, what);
  snprintf(what, sizeof what, "uint64: %ld divisions match", n);
  QT_CHECK(bad64 == 0, what);
  snprintf(what, sizeof what, "size_t: %ld divisions match", n);
  QT_CHECK(badsz == 0, what);
  QT_END();
}
