/* libltdl (libtool's portable dlopen): open QNX's libm through ltdl, look up
 * cos() and sqrt() and call them for known values, check module info, the
 * user search path round trip, and that a missing module and a missing
 * symbol both fail with an lt_dlerror() message. */
#include "common.h"
#include <math.h>
#include <ltdl.h>
typedef double (*d2d)(double);
int main(void) {
  QT_BEGIN("libtool");
  QT_CHECK(lt_dlinit() == 0, "lt_dlinit");
  QT_CHECK(lt_dlsetsearchpath("/tmp/qnxport-ltdl-a") == 0 && lt_dladdsearchdir("/tmp/qnxport-ltdl-b") == 0,
           "set + add search directories");
  const char *sp = lt_dlgetsearchpath();
  printf("  info search path: %s\n", sp ? sp : "(null)");
  QT_CHECK(sp && strcmp(sp, "/tmp/qnxport-ltdl-a:/tmp/qnxport-ltdl-b") == 0, "search path reads back");

  static const char *cands[] = { "libm.so.3", "/lib/libm.so.3", "/usr/lib/libm.so.3", "/proc/boot/libm.so.3", "libm.so", NULL };
  lt_dlhandle h = NULL; const char *used = NULL;
  for (int i = 0; cands[i] && !h; i++) { h = lt_dlopen(cands[i]); if (h) used = cands[i]; }
  printf("  info opened %s\n", used ? used : "(nothing)");
  QT_CHECK(h != NULL, "lt_dlopen libm");
  if (h) {
    d2d c = (d2d)lt_dlsym(h, "cos"), s = (d2d)lt_dlsym(h, "sqrt");
    QT_CHECK(c && s, "lt_dlsym cos + sqrt");
    if (c && s) {
      double v1 = c(0.0), v2 = s(2.0), v3 = c(M_PI);
      printf("  info cos(0)=%.17g sqrt(2)=%.17g cos(pi)=%.17g\n", v1, v2, v3);
      QT_CHECK(v1 == 1.0 && fabs(v2 - 1.4142135623730951) < 1e-15 && fabs(v3 + 1.0) < 1e-15,
               "called through ltdl: known values");
    }
    const lt_dlinfo *inf = lt_dlgetinfo(h);
    QT_CHECK(inf && inf->ref_count >= 1, "lt_dlgetinfo: module refcount");
    if (inf) printf("  info filename=%s name=%s refs=%d\n", inf->filename ? inf->filename : "(null)",
                    inf->name ? inf->name : "(null)", inf->ref_count);
    lt_dlerror();
    QT_CHECK(lt_dlsym(h, "qnx_no_such_symbol_xyz") == NULL, "missing symbol -> NULL");
    const char *e = lt_dlerror();
    QT_CHECK(e != NULL, "lt_dlerror explains a missing symbol");
    if (e) printf("  info %s\n", e);
    QT_CHECK(lt_dlclose(h) == 0, "lt_dlclose");
  }
  lt_dlerror();
  QT_CHECK(lt_dlopenext("/tmp/qnxport-ltdl-none/libnothere") == NULL, "missing module -> NULL");
  const char *e = lt_dlerror();
  QT_CHECK(e != NULL && e[0], "lt_dlerror explains a missing module");
  if (e) printf("  info %s\n", e);
  QT_CHECK(lt_dlexit() == 0, "lt_dlexit");
  QT_END();
}
