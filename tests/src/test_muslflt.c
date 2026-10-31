/* muslflt: musl's libm, complex, fenv and float formatting/parsing, built
 * as a separate library. Every function is looked up IN libmuslflt.so.1
 * (dlsym on its handle), so the checks exercise musl's code, not the QNX
 * libm of the same names: correctly rounded results, exact special cases,
 * rounding-mode control through its fesetround, complex functions, and
 * printf/strtod round trips of tricky values. */
#include "common.h"
#include <complex.h>
#include <dlfcn.h>
#include <errno.h>
#include <fenv.h>
#include <math.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdlib.h>
static void *h;
static int missing;
static int foreign;
static void *sym(const char *n) {
  void *p = dlsym(h, n);
  if (!p) { printf("  info missing symbol %s\n", n); missing++; return p; }
  /* the definition must be muslflt's own, not QNX libc/libm's */
  Dl_info di;
  if (dladdr(p, &di) && di.dli_fname && !strstr(di.dli_fname, "muslflt")) {
    printf("  info %s resolved to %s, not libmuslflt\n", n, di.dli_fname); foreign++;
  }
  return p;
}
typedef double (*d1)(double);
typedef double (*d2)(double, double);
typedef double (*d3)(double, double, double);
static double D1(const char *n, double x) { d1 f = (d1)sym(n); return f ? f(x) : NAN; }
static double D2(const char *n, double x, double y) { d2 f = (d2)sym(n); return f ? f(x, y) : NAN; }
static int ulp1(double a, double b) {
  int64_t ia, ib; memcpy(&ia, &a, 8); memcpy(&ib, &b, 8);
  int ok = (ia == ib) || llabs(ia - ib) <= 1;
  if (!ok) printf("  info got %.17g want %.17g\n", a, b);
  return ok;
}
int main(void) {
  QT_BEGIN("muslflt");
  h = dlopen("libmuslflt.so.1", RTLD_NOW | RTLD_LOCAL);
  if (!h) { const char *e = dlerror(); printf("  info dlopen: %s\n", e ? e : "?"); }
  QT_CHECK(h != NULL, "dlopen libmuslflt.so.1");
  if (!h) QT_END();

  QT_CHECK(D1("sqrt", 2.0) == 1.4142135623730951, "sqrt(2) correctly rounded");
  QT_CHECK(ulp1(D1("exp", 1.0), 2.718281828459045), "exp(1)");
  QT_CHECK(ulp1(D1("log", 10.0), 2.302585092994046), "log(10)");
  QT_CHECK(ulp1(D1("sin", 1.0), 0.8414709848078965) && ulp1(D1("cos", 1.0), 0.5403023058681398), "sin(1), cos(1)");
  QT_CHECK(ulp1(D1("sin", 1e22), -0.8522008497671888), "sin(1e22) (Payne-Hanek reduction)");
  QT_CHECK(D2("pow", 2.0, 10.0) == 1024.0 && D2("pow", -8.0, 1.0 / 3) != D2("pow", -8.0, 1.0 / 3), "pow(2,10)=1024, pow(-8,1/3)=NaN");
  QT_CHECK(D1("cbrt", -27.0) == -3.0 && D2("hypot", 3.0, 4.0) == 5.0, "cbrt(-27) = -3, hypot(3,4) = 5");
  QT_CHECK(ulp1(D1("tgamma", 5.0), 24.0) && ulp1(D1("lgamma", 0.5), 0.5723649429247001), "tgamma(5), lgamma(1/2) = log(sqrt(pi))");
  QT_CHECK(ulp1(D1("erf", 0.5), 0.5204998778130465), "erf(0.5)");
  d3 fma_ = (d3)sym("fma");
  QT_CHECK(fma_ && fma_(0.1, 10.0, -1.0) == 5.551115123125783e-17, "fma(0.1, 10, -1) is fused (5.55e-17)");
  QT_CHECK(D1("rint", 2.5) == 2.0 && D1("round", 2.5) == 3.0 && D1("trunc", -2.7) == -2.0 && D1("floor", -2.5) == -3.0,
           "rint/round/trunc/floor");
  QT_CHECK(D2("fmod", 10.5, 3.0) == 1.5 && D2("remainder", 10.5, 3.0) == -1.5, "fmod and remainder");
  QT_CHECK(isnan(D1("log", -1.0)) && isinf(D1("exp", 1000.0)) && D1("exp", -1000.0) == 0.0, "special values");

  int (*setr)(int) = (int (*)(int))sym("fesetround");
  int (*getr)(void) = (int (*)(void))sym("fegetround");
  if (setr && getr) {
    QT_CHECK(setr(FE_UPWARD) == 0 && getr() == FE_UPWARD, "fesetround(FE_UPWARD)");
    double up = D1("rint", 2.1), upn = D1("rint", -2.9);
    setr(FE_DOWNWARD);
    double dn = D1("rint", 2.9);
    setr(FE_TONEAREST);
    QT_CHECK(up == 3.0 && upn == -2.0 && dn == 2.0 && getr() == FE_TONEAREST, "rint follows the rounding mode");
  }

  double complex (*cexp_)(double complex) = (double complex (*)(double complex))sym("cexp");
  double complex (*csqrt_)(double complex) = (double complex (*)(double complex))sym("csqrt");
  double (*cabs_)(double complex) = (double (*)(double complex))sym("cabs");
  if (cexp_ && csqrt_ && cabs_) {
    double complex e = cexp_(I * 3.141592653589793);
    printf("  info cexp(i*pi) = %.17g %+.3g i\n", creal(e), cimag(e));
    QT_CHECK(creal(e) == -1.0 && fabs(cimag(e)) < 1.3e-16, "cexp(i*pi) = -1 (Euler)");
    double complex s = csqrt_(-4.0 + 0.0 * I);
    QT_CHECK(creal(s) == 0.0 && cimag(s) == 2.0, "csqrt(-4) = 2i");
    QT_CHECK(cabs_(3.0 + 4.0 * I) == 5.0, "cabs(3+4i) = 5");
  }

  int (*snp)(char *, size_t, const char *, ...) = (int (*)(char *, size_t, const char *, ...))sym("snprintf");
  double (*strtod_)(const char *, char **) = (double (*)(const char *, char **))sym("strtod");
  float (*strtof_)(const char *, char **) = (float (*)(const char *, char **))sym("strtof");
  if (snp && strtod_ && strtof_) {
    char b[128];
    snp(b, sizeof b, "%.17g|%a|%.3e|%g|%08.2f", 0.1, 1.0, 12345.678, 1e-5, -3.14159);
    printf("  info snprintf -> %s\n", b);
    QT_CHECK(strcmp(b, "0.10000000000000001|0x1p+0|1.235e+04|1e-05|-0003.14") == 0, "snprintf float formats");
    snp(b, sizeof b, "%.20f", 0.5);
    QT_CHECK(strcmp(b, "0.50000000000000000000") == 0, "snprintf %.20f");
    snp(b, sizeof b, "%.0f %.0f %f", 0.5, 1.5, (double)INFINITY);
    QT_CHECK(strcmp(b, "0 2 inf") == 0, "snprintf ties-to-even and inf");
    char *end = NULL;
    QT_CHECK(strtod_("0x1.8p1xyz", &end) == 3.0 && end && strcmp(end, "xyz") == 0, "strtod hex float, end pointer");
    QT_CHECK(strtod_("2.2250738585072011e-308", NULL) == 2.2250738585072011e-308, "strtod near DBL_MIN (correct rounding)");
    QT_CHECK(strtod_("9007199254740993", NULL) == 9007199254740992.0, "strtod 2^53+1 rounds to even");
    errno = 0;
    double tiny = strtod_("1e-400", NULL);
    QT_CHECK(tiny == 0.0 && errno == ERANGE, "strtod underflow -> 0, ERANGE");
    QT_CHECK(strtof_("3.4028235e38", NULL) == 3.4028235e38f, "strtof FLT_MAX");
    int rt = 1;
    for (int i = 1; i < 200; i++) {
      double v = (double)i / 7.0 * pow(10.0, (i % 40) - 20);
      snp(b, sizeof b, "%.17g", v);
      if (strtod_(b, NULL) != v) rt = 0;
    }
    QT_CHECK(rt, "%.17g -> strtod round trips 199 values exactly");
  }
  QT_CHECK(missing == 0, "all looked-up symbols present");
  QT_CHECK(foreign == 0, "every looked-up symbol is defined in libmuslflt itself");
  dlclose(h);
  QT_END();
}
