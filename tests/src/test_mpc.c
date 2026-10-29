/* MPC: arbitrary-precision complex arithmetic on top of MPFR. */
#include "common.h"
#include <stdlib.h>
#include <mpc.h>
int main(void) {
  QT_BEGIN("mpc");
  printf("  info MPC %s\n", mpc_get_version());
  mpc_t z, w, r;
  mpc_init2(z, 128); mpc_init2(w, 128); mpc_init2(r, 128);
  mpc_set_ui_ui(z, 3, 4, MPC_RNDNN);            /* 3 + 4i */
  mpfr_t abs; mpfr_init2(abs, 128);
  mpc_abs(abs, z, MPFR_RNDN);
  QT_CHECK(mpfr_cmp_ui(abs, 5) == 0, "|3+4i| == 5");
  mpc_sqr(w, z, MPC_RNDNN);                      /* (3+4i)^2 = -7 + 24i */
  QT_CHECK(mpfr_cmp_si(mpc_realref(w), -7) == 0 &&
           mpfr_cmp_si(mpc_imagref(w), 24) == 0, "(3+4i)^2 == -7+24i");
  /* e^(i*pi) + 1 == 0, to the working precision. */
  mpfr_t pi; mpfr_init2(pi, 128); mpfr_const_pi(pi, MPFR_RNDN);
  mpc_set_ui(r, 0, MPC_RNDNN);
  mpfr_set(mpc_imagref(r), pi, MPFR_RNDN);
  mpc_exp(r, r, MPC_RNDNN);
  mpc_add_ui(r, r, 1, MPC_RNDNN);
  mpc_abs(abs, r, MPFR_RNDN);
  QT_CHECK(mpfr_cmp_d(abs, 1e-30) < 0, "|e^(i*pi) + 1| is essentially zero");
  mpfr_clear(abs); mpfr_clear(pi);
  mpc_clear(z); mpc_clear(w); mpc_clear(r);
  QT_END();
}
