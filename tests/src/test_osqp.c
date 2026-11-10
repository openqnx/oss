/* osqp: OSQP's own two-variable demo QP, solved to its known optimum:
 *   minimise 1/2 x'[4 1; 1 2]x + [1 1]x  s.t.  x1 + x2 = 1, 0 <= x1, x2 <= 0.7
 * has x* = (0.3, 0.7) and objective 1.88. Then the same problem with a
 * contradictory constraint (x1 + x2 >= 2) must be reported infeasible. */
#include "common.h"
#include <math.h>
#include <stdlib.h>
#include <osqp/osqp.h>

static int solve(c_float l0, c_float u0, OSQPWorkspace **w) {
  static c_float P_x[3] = {4.0, 1.0, 2.0}; static c_int P_i[3] = {0, 0, 1}, P_p[3] = {0, 1, 3};
  static c_float q[2] = {1.0, 1.0};
  static c_float A_x[4] = {1.0, 1.0, 1.0, 1.0}; static c_int A_i[4] = {0, 1, 0, 2}, A_p[3] = {0, 2, 4};
  static c_float l[3], u[3];
  l[0] = l0; l[1] = 0.0; l[2] = 0.0; u[0] = u0; u[1] = 0.7; u[2] = 0.7;
  OSQPSettings *s = (OSQPSettings *)c_malloc(sizeof(OSQPSettings));
  OSQPData *d = (OSQPData *)c_malloc(sizeof(OSQPData));
  d->n = 2; d->m = 3;
  d->P = csc_matrix(2, 2, 3, P_x, P_i, P_p); d->q = q;
  d->A = csc_matrix(3, 2, 4, A_x, A_i, A_p); d->l = l; d->u = u;
  osqp_set_default_settings(s);
  s->alpha = 1.0; s->verbose = 0; s->eps_abs = 1e-6; s->eps_rel = 1e-6; s->polish = 1;
  c_int e = osqp_setup(w, d, s);
  if (e == 0) osqp_solve(*w);
  c_free(d->A); c_free(d->P); c_free(d); c_free(s);
  return (int)e;
}

int main(void) {
  QT_BEGIN("osqp");
  OSQPWorkspace *w = NULL; char m[160];
  QT_CHECK(solve(1.0, 1.0, &w) == 0, "osqp_setup of the demo problem");
  if (w) {
    QT_CHECK(w->info->status_val == OSQP_SOLVED, "status: solved");
    snprintf(m, sizeof m, "x* = (%.6f, %.6f), expected (0.3, 0.7)", w->solution->x[0], w->solution->x[1]);
    QT_CHECK(fabs(w->solution->x[0] - 0.3) < 1e-4 && fabs(w->solution->x[1] - 0.7) < 1e-4, m);
    snprintf(m, sizeof m, "objective %.6f, expected 1.88", w->info->obj_val);
    QT_CHECK(fabs(w->info->obj_val - 1.88) < 1e-4, m);
    osqp_cleanup(w);
  }
  w = NULL;
  QT_CHECK(solve(2.0, 3.0, &w) == 0, "osqp_setup of the infeasible variant");
  if (w) {
    snprintf(m, sizeof m, "x1 + x2 >= 2 with x <= 0.7: status %s", w->info->status);
    QT_CHECK(w->info->status_val == OSQP_PRIMAL_INFEASIBLE, m);
    osqp_cleanup(w);
  }
  QT_END();
}
