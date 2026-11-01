/* libtommath: multiple-precision arithmetic with known answers --
 * radix conversion, multiplication, division, modular exponentiation,
 * gcd, modular inverse, integer square root and a primality test. */
#include <tommath.h>
#include "common.h"

static int eq(const mp_int *a, const char *dec)
{
  mp_int b;
  int r;
  if (mp_init(&b) != MP_OKAY) return 0;
  r = mp_read_radix(&b, dec, 10) == MP_OKAY && mp_cmp(a, &b) == MP_EQ;
  mp_clear(&b);
  return r;
}

int main(void)
{
  mp_int a, b, c, d;
  char buf[256];
  mp_bool prime = MP_NO;
  QT_BEGIN("libtommath");
  QT_CHECK(mp_init_multi(&a, &b, &c, &d, NULL) == MP_OKAY, "mp_init_multi");

  /* 2^128 */
  QT_CHECK(mp_2expt(&a, 128) == MP_OKAY && eq(&a, "340282366920938463463374607431768211456"), "2^128 = 340282366920938463463374607431768211456");
  QT_CHECK(mp_to_radix(&a, buf, sizeof buf, NULL, 16) == MP_OKAY && strcmp(buf, "100000000000000000000000000000000") == 0, "2^128 in hex");

  /* 12345678901234567890 * 98765432109876543210 */
  mp_read_radix(&a, "12345678901234567890", 10);
  mp_read_radix(&b, "98765432109876543210", 10);
  QT_CHECK(mp_mul(&a, &b, &c) == MP_OKAY && eq(&c, "1219326311370217952237463801111263526900"), "multiplication");
  QT_CHECK(mp_div(&c, &a, &d, NULL) == MP_OKAY && mp_cmp(&d, &b) == MP_EQ, "division undoes it");

  /* 4^13 mod 497 = 445 (the textbook example) */
  mp_set(&a, 4); mp_set(&b, 13); mp_set(&c, 497);
  QT_CHECK(mp_exptmod(&a, &b, &c, &d) == MP_OKAY && eq(&d, "445"), "4^13 mod 497 = 445");

  /* gcd(1071, 462) = 21; 3^-1 mod 11 = 4 */
  mp_set(&a, 1071); mp_set(&b, 462);
  QT_CHECK(mp_gcd(&a, &b, &c) == MP_OKAY && eq(&c, "21"), "gcd(1071, 462) = 21");
  mp_set(&a, 3); mp_set(&b, 11);
  QT_CHECK(mp_invmod(&a, &b, &c) == MP_OKAY && eq(&c, "4"), "3^-1 mod 11 = 4");

  /* sqrt(10^40) = 10^20 */
  mp_read_radix(&a, "10000000000000000000000000000000000000000", 10);
  QT_CHECK(mp_sqrt(&a, &b) == MP_OKAY && eq(&b, "100000000000000000000"), "sqrt(10^40) = 10^20");

  /* 2^127 - 1 is a Mersenne prime; 2^127 + 1 is divisible by 3 */
  mp_2expt(&a, 127); mp_sub_d(&a, 1, &a);
  QT_CHECK(mp_prime_is_prime(&a, 16, &prime) == MP_OKAY && prime == MP_YES, "2^127-1 is prime");
  mp_add_d(&a, 2, &a);
  QT_CHECK(mp_prime_is_prime(&a, 16, &prime) == MP_OKAY && prime == MP_NO, "2^127+1 is composite");

  mp_clear_multi(&a, &b, &c, &d, NULL);
  QT_END();
}
