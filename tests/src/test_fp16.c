/* fp16 (header-only): IEEE half <-> single conversions with known bit
 * patterns, round-to-nearest-even on ties, subnormals, infinities and NaN,
 * a round trip of every finite half value, and the ARM "alternative" half
 * format (no infinities: 65504 is not its maximum). */
#include "common.h"
#include <fp16.h>
#include <math.h>
#include <stdint.h>

int main(void) {
  QT_BEGIN("fp16");
  struct { float f; uint16_t h; } k[] = {
    { 1.0f, 0x3C00 }, { -2.0f, 0xC000 }, { 0.5f, 0x3800 }, { 65504.0f, 0x7BFF },
    { 0.0f, 0x0000 }, { -0.0f, 0x8000 }, { 6.103515625e-05f, 0x0400 },   /* smallest normal */
    { 5.9604644775390625e-08f, 0x0001 },                                 /* smallest subnormal */
    { 1.0009765625f, 0x3C01 },                                           /* 1 + 2^-10 */
    { 1.00048828125f, 0x3C00 },                                          /* 1 + 2^-11: tie, to even */
    { 1.00146484375f, 0x3C02 },                                          /* 1 + 3*2^-11: tie, to even */
    { 65520.0f, 0x7C00 },                                                /* rounds to infinity */
    { 1e-9f, 0x0000 },                                                   /* underflows to zero */
  };
  char what[128];
  int bad = 0;
  for (size_t i = 0; i < sizeof k / sizeof k[0]; i++) {
    uint16_t h = fp16_ieee_from_fp32_value(k[i].f);
    if (h != k[i].h) { printf("  info %.10g -> 0x%04X, expected 0x%04X\n", k[i].f, h, k[i].h); bad++; }
  }
  snprintf(what, sizeof what, "fp32 -> fp16: %zu known bit patterns", sizeof k / sizeof k[0]);
  QT_CHECK(bad == 0, what);
  QT_CHECK(fp16_ieee_to_fp32_value(0x3555) == 0.333251953125f, "fp16 0x3555 -> 0.333251953125");
  QT_CHECK(fp16_ieee_to_fp32_value(0x0001) == ldexpf(1.0f, -24), "fp16 0x0001 -> 2^-24 (subnormal)");
  QT_CHECK(fp16_ieee_to_fp32_value(0xFBFF) == -65504.0f, "fp16 0xFBFF -> -65504");
  QT_CHECK(isinf(fp16_ieee_to_fp32_value(0x7C00)) && fp16_ieee_to_fp32_value(0xFC00) < 0, "fp16 +/-inf");
  QT_CHECK(isnan(fp16_ieee_to_fp32_value(0x7E00)) && (fp16_ieee_from_fp32_value(NAN) & 0x7C00) == 0x7C00
           && (fp16_ieee_from_fp32_value(NAN) & 0x03FF) != 0, "NaN both ways");
  long rt = 0, n = 0;
  for (uint32_t h = 0; h < 0x10000; h++) {
    if ((h & 0x7C00) == 0x7C00 && (h & 0x03FF)) continue;     /* NaNs */
    n++;
    if (fp16_ieee_from_fp32_value(fp16_ieee_to_fp32_value((uint16_t)h)) == h) rt++;
  }
  snprintf(what, sizeof what, "round trip of every non-NaN half: %ld/%ld", rt, n);
  QT_CHECK(rt == n && n == 63490, what);
  /* raw 32-bit form as well */
  QT_CHECK(fp16_ieee_to_fp32_bits(0x3C00) == 0x3F800000u, "fp16_ieee_to_fp32_bits(1.0) = 0x3F800000");
  /* alternative half: exponent 31 is a normal binade */
  QT_CHECK(fp16_alt_to_fp32_value(0x7C00) == 65536.0f && fp16_alt_to_fp32_value(0x7FFF) == 131008.0f,
           "alt half: 0x7C00 = 65536, max 131008");
  QT_CHECK(fp16_alt_from_fp32_value(1.0f) == 0x3C00 && fp16_alt_from_fp32_value(100000.0f) == 0x7E1A,
           "alt half: 1.0 -> 0x3C00, 100000 -> 0x7E1A");
  QT_END();
}
