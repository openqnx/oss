/* MPFR: arbitrary-precision floating point with correct rounding. */
#include "common.h"
#include <stdlib.h>
#include <mpfr.h>
int main(void) {
  QT_BEGIN("mpfr");
  printf("  info MPFR %s\n", mpfr_get_version());
  mpfr_t pi, e, sum;
  mpfr_inits2(200, pi, e, sum, (mpfr_ptr)0);
  mpfr_const_pi(pi, MPFR_RNDN);
  char buf[128];
  mpfr_snprintf(buf, sizeof buf, "%.30Rf", pi);
  /* Compare 28 decimals, not 30: pi is 3.14159265358979323846264338327950…,
   * so the 30th place is a rounding boundary and a correctly rounded MPFR
   * prints …280 where the truncated expansion reads …279. */
  QT_CHECK(strncmp(buf, "3.1415926535897932384626433832", 30) == 0,
           "pi to 28 decimals matches the known value");
  printf("  info pi = %s\n", buf);
  mpfr_set_ui(e, 1, MPFR_RNDN);
  mpfr_exp(e, e, MPFR_RNDN);
  mpfr_snprintf(buf, sizeof buf, "%.20Rf", e);
  QT_CHECK(strncmp(buf, "2.71828182845904523536", 22) == 0, "e to 20 decimals");
  mpfr_add(sum, pi, e, MPFR_RNDN);
  QT_CHECK(mpfr_cmp(sum, pi) > 0 && mpfr_cmp(sum, e) > 0, "pi + e exceeds both");
  /* Correct rounding is the whole point of MPFR: 1/3 at 10 bits. */
  mpfr_t third; mpfr_init2(third, 10);
  mpfr_set_ui(third, 1, MPFR_RNDN);
  mpfr_div_ui(third, third, 3, MPFR_RNDN);
  QT_CHECK(mpfr_get_prec(third) == 10, "precision is honoured");
  QT_CHECK(mpfr_get_d(third, MPFR_RNDN) > 0.333 && mpfr_get_d(third, MPFR_RNDN) < 0.334,
           "1/3 at 10 bits is close to 0.3333");
  mpfr_clear(third);
  mpfr_clears(pi, e, sum, (mpfr_ptr)0);
  mpfr_free_cache();
  QT_END();
}
