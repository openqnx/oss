/* swig, C target language: test_swig_example.i was turned into c_wrap.c and
 * c_wrap.h at build time by swig -c (SWIG 4.3.1, the version ported); this
 * program calls the library only through the generated wrapper functions
 * (qnxswig_<name>: the module prefix, the qs_ prefix removed by the
 * interface's PCRE2 %rename) and checks the known answers. */
#include "common.h"
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include "c_wrap.h"

int main(void) {
  char *r;
  qs_acc a = { 0, 0 };
  QT_BEGIN("swig_c");
  QT_CHECK(qnxswig_gcd(84, 36) == 12 && qnxswig_gcd(-17, 5) == 1, "qnxswig_gcd: gcd(84,36)=12, gcd(-17,5)=1");
  QT_CHECK(fabs(qnxswig_hypot3(2, 3, 6) - 7.0) < 1e-12, "qnxswig_hypot3(2,3,6) = 7");
  QT_CHECK(qnxswig_crc8("123456789") == 0xF4, "qnxswig_crc8(\"123456789\") = 0xF4 (CRC-8 check value)");
  r = qnxswig_reverse("QNX swig");
  QT_CHECK(r && strcmp(r, "giws XNQ") == 0, "qnxswig_reverse: a new string");
  free(r);
  qnxswig_acc_add(&a, 5); qnxswig_acc_add(&a, 7);
  QT_CHECK(a.n == 2 && a.total == 12, "qnxswig_acc_add on the struct: n=2 total=12");
  QT_CHECK(qs_calls == 7, "global variable qs_calls counted 7 calls");
  QT_END();
}
