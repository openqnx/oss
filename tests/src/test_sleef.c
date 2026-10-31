/* SLEEF: scalar double/float functions against correctly rounded reference
 * values (within the documented 1.0 / 0.5 ULP bounds, checked as a few
 * ULP), sincos, special values, and the 2-wide double / 4-wide float SIMD
 * entry points (SSE2 on x86_64, AdvSIMD on aarch64), which dispatch to the
 * best kernel for the CPU at run time. */
#include "common.h"
#include <math.h>
#include <stdint.h>
#include <sleef.h>
static int64_t ulps(double a, double b) {
  int64_t ia, ib; memcpy(&ia, &a, 8); memcpy(&ib, &b, 8);
  if (ia < 0) ia = INT64_MIN - ia;
  if (ib < 0) ib = INT64_MIN - ib;
  return ia > ib ? ia - ib : ib - ia;
}
static int near(const char *what, double got, double want, int maxulp) {
  int64_t u = ulps(got, want);
  if (u > maxulp) printf("  info %s = %.17g, want %.17g (%lld ulp)\n", what, got, want, (long long)u);
  return u <= maxulp;
}
static int nearf(float got, float want, int maxulp) {
  int32_t a, b; memcpy(&a, &got, 4); memcpy(&b, &want, 4);
  int32_t d = a > b ? a - b : b - a;
  return d <= maxulp;
}
int main(void) {
  QT_BEGIN("sleef");
  QT_CHECK(near("sin(1)", Sleef_sin_u10(1.0), 0.8414709848078965, 1), "sin(1)");
  QT_CHECK(near("cos(1)", Sleef_cos_u10(1.0), 0.5403023058681398, 1), "cos(1)");
  QT_CHECK(near("sin(1e6)", Sleef_sin_u10(1e6), -0.34999350217129294, 1), "sin(1e6) (large-argument reduction)");
  QT_CHECK(near("exp(1)", Sleef_exp_u10(1.0), 2.718281828459045, 1), "exp(1) = e");
  QT_CHECK(near("log(10)", Sleef_log_u10(10.0), 2.302585092994046, 1), "log(10)");
  QT_CHECK(near("pow(2,0.5)", Sleef_pow_u10(2.0, 0.5), 1.4142135623730951, 1), "pow(2, 0.5) = sqrt 2");
  QT_CHECK(Sleef_pow_u10(3.0, 4.0) == 81.0, "pow(3, 4) = 81 exactly");
  QT_CHECK(near("atan2(1,1)", 4 * Sleef_atan2_u10(1.0, 1.0), 3.141592653589793, 2), "4*atan2(1,1) = pi");
  QT_CHECK(near("sinpi(1/6)", Sleef_sinpi_u05(1.0 / 6), 0.5, 1), "sinpi(1/6) = 0.5");
  QT_CHECK(near("tgamma(5)", Sleef_tgamma_u10(5.0), 24.0, 1), "tgamma(5) = 24");
  QT_CHECK(near("erf(0.5)", Sleef_erf_u10(0.5), 0.5204998778130465, 1), "erf(0.5)");
  QT_CHECK(near("cbrt(27)", Sleef_cbrt_u10(27.0), 3.0, 1), "cbrt(27) = 3");
  Sleef_double2 sc = Sleef_sincos_u10(0.5);
  QT_CHECK(near("sincos.x", sc.x, 0.479425538604203, 1) && near("sincos.y", sc.y, 0.8775825618903728, 1), "sincos(0.5)");
  QT_CHECK(isnan(Sleef_log_u10(-1.0)) && isinf(Sleef_exp_u10(1000.0)) && Sleef_exp_u10(-1000.0) == 0.0,
           "special values: log(-1)=NaN, exp overflow/underflow");
  QT_CHECK(nearf(Sleef_sinf_u10(1.0f), 0.84147098f, 1) && nearf(Sleef_expf_u10(2.0f), 7.3890561f, 1), "float sinf/expf");

  double din[2] = { 0.25, 2.0 }, dout[2], eout[2];
  float fin[4] = { 0.5f, 1.0f, 2.0f, 4.0f }, fout[4];
#if defined(__x86_64__)
  _mm_storeu_pd(dout, Sleef_sind2_u10(_mm_loadu_pd(din)));
  _mm_storeu_pd(eout, Sleef_expd2_u10(_mm_loadu_pd(din)));
  _mm_storeu_ps(fout, Sleef_logf4_u10(_mm_loadu_ps(fin)));
  printf("  info SIMD: SSE2 entry points\n");
#elif defined(__aarch64__)
  vst1q_f64(dout, Sleef_sind2_u10(vld1q_f64(din)));
  vst1q_f64(eout, Sleef_expd2_u10(vld1q_f64(din)));
  vst1q_f32(fout, Sleef_logf4_u10(vld1q_f32(fin)));
  printf("  info SIMD: AdvSIMD entry points\n");
#endif
  QT_CHECK(near("sind2[0]", dout[0], 0.24740395925452294, 1) && near("sind2[1]", dout[1], 0.9092974268256817, 1),
           "Sleef_sind2_u10 (2 lanes)");
  QT_CHECK(near("expd2[0]", eout[0], 1.2840254166877414, 1) && near("expd2[1]", eout[1], 7.38905609893065, 1),
           "Sleef_expd2_u10 (2 lanes)");
  QT_CHECK(nearf(fout[0], -0.69314718f, 1) && fout[1] == 0.0f && nearf(fout[2], 0.69314718f, 1) && nearf(fout[3], 1.3862944f, 1),
           "Sleef_logf4_u10 (4 lanes)");
  QT_END();
}
