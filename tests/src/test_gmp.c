/* GMP: bignum arithmetic and a primality test. */
#include "common.h"
#include <stdlib.h>
#include <gmp.h>
int main(void) {
  QT_BEGIN("gmp");
  printf("  info GMP %s (%d-bit limbs)\n", gmp_version, (int)mp_bits_per_limb);
  mpz_t a, b, c;
  mpz_inits(a, b, c, NULL);
  mpz_set_ui(a, 2);
  mpz_pow_ui(b, a, 127);
  mpz_sub_ui(b, b, 1);               /* 2^127 - 1, a Mersenne prime */
  QT_CHECK(mpz_sizeinbase(b, 2) == 127, "2^127-1 is 127 bits wide");
  QT_CHECK(mpz_probab_prime_p(b, 30) != 0, "2^127-1 is prime");
  char *s = mpz_get_str(NULL, 10, b);
  QT_CHECK(s && strcmp(s, "170141183460469231731687303715884105727") == 0,
           "decimal conversion matches the known value");
  free(s);
  mpz_set_str(c, "123456789012345678901234567890", 10);
  mpz_mul(a, c, c);
  mpz_sqrt(b, a);
  QT_CHECK(mpz_cmp(b, c) == 0, "sqrt(n^2) == n for a 30-digit n");
  mpz_clears(a, b, c, NULL);
  QT_END();
}
