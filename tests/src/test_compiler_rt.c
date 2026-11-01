/* compiler-rt: functions of LLVM's builtins library with known values.
 *
 * libclang_rt.builtins.a is linked ahead of GCC's libgcc, so the 128-bit
 * division and bit-count helpers both provide resolve here; __mulodi4 and
 * __muloti4 (multiply with overflow flag) exist only in compiler-rt, so the
 * test does not link without it. Values are checked against exact answers. */
#include "common.h"
#include <stdint.h>

typedef unsigned __int128 tu_int;
typedef __int128 ti_int;
tu_int __udivti3(tu_int a, tu_int b);
tu_int __umodti3(tu_int a, tu_int b);
ti_int __divti3(ti_int a, ti_int b);
int __popcountdi2(int64_t a);
int __clzdi2(int64_t a);
int __ctzdi2(int64_t a);
int64_t __mulodi4(int64_t a, int64_t b, int *overflow);
ti_int __muloti4(ti_int a, ti_int b, int *overflow);
uint32_t __bswapsi2(uint32_t u);
double __floatuntidf(tu_int a);
int64_t __ashldi3(int64_t a, int b);

#define U128(hi, lo) (((tu_int)(hi) << 64) | (tu_int)(lo))

int main(void) {
  QT_BEGIN("compiler-rt");
  /* 2^100 + 12345 divided by 1000000007 */
  tu_int a = U128(0x1000000000ull, 12345), b = 1000000007;
  tu_int q = __udivti3(a, b), r = __umodti3(a, b);
  QT_CHECK(q * b + r == a && r < b, "__udivti3/__umodti3: q*b + r == a, r < b");
  /* 1267650591354675262013 = 68 * 2^64 + 13271994342425752125, remainder 976383630 */
  QT_CHECK((uint64_t)(q >> 64) == 68 && (uint64_t)q == 13271994342425752125ull && (uint64_t)r == 976383630ull,
           "(2^100+12345) / 1000000007 = 1267650591354675262013 r 976383630");
  QT_CHECK(__udivti3(U128(0xFFFFFFFFFFFFFFFFull, 0xFFFFFFFFFFFFFFFFull), 3) ==
           U128(0x5555555555555555ull, 0x5555555555555555ull), "(2^128-1) / 3 = 0x5555...5555");
  QT_CHECK(__divti3(-(ti_int)1000000, 7) == -142857, "__divti3(-1000000, 7) = -142857 (truncates)");

  QT_CHECK(__popcountdi2(0) == 0 && __popcountdi2(-1) == 64 && __popcountdi2(0x00F0F0F0F0F0F0F0ll) == 28,
           "__popcountdi2: 0 -> 0, -1 -> 64, 0x00F0F0F0F0F0F0F0 -> 28");
  QT_CHECK(__clzdi2(1) == 63 && __clzdi2(0x0000800000000000ll) == 16, "__clzdi2: 1 -> 63, 2^47 -> 16");
  QT_CHECK(__ctzdi2(0x100) == 8 && __ctzdi2(INT64_MIN) == 63, "__ctzdi2: 0x100 -> 8, 2^63 -> 63");
  QT_CHECK(__bswapsi2(0x11223344u) == 0x44332211u, "__bswapsi2(0x11223344) = 0x44332211");
  QT_CHECK(__ashldi3(3, 40) == 3298534883328ll, "__ashldi3(3, 40) = 3298534883328");

  int ov = -1;
  QT_CHECK(__mulodi4(3037000499ll, 3037000499ll, &ov) == 9223372030926249001ll && ov == 0,
           "__mulodi4(3037000499^2) = 9223372030926249001, no overflow");
  ov = 0;
  (void)__mulodi4(3037000500ll, 3037000500ll, &ov);
  QT_CHECK(ov == 1, "__mulodi4(3037000500^2) overflows int64");
  ov = -1;
  ti_int m = __muloti4((ti_int)INT64_MAX, (ti_int)INT64_MAX, &ov);
  QT_CHECK(ov == 0 && (uint64_t)(m >> 64) == 0x3FFFFFFFFFFFFFFFull && (uint64_t)m == 1,
           "__muloti4(INT64_MAX^2) = 0x3FFF...FFFF_0000...0001, no overflow");
  ov = 0;
  (void)__muloti4(U128(0x4000000000000000ull, 0), 2, &ov);
  QT_CHECK(ov == 1, "__muloti4(2^126 * 2) overflows int128");
  QT_CHECK(__floatuntidf(U128(1, 0)) == 18446744073709551616.0, "__floatuntidf(2^64) = 18446744073709551616.0");
  QT_END();
}
