/* libltdl: libtool's portable dlopen wrapper. QNX has no libdl -- the dynamic
 * loader lives in libc -- so this exercises the path that broke on every other
 * package that reached for -ldl. */
#include "common.h"
#include <stdlib.h>
#include <ltdl.h>
int main(void) {
  QT_BEGIN("libltdl");
  QT_CHECK(lt_dlinit() == 0, "lt_dlinit");
  /* Open a library we staged ourselves and call through it. */
  lt_dlhandle h = lt_dlopenext("libz");
  if (!h) h = lt_dlopen("libz.so.1");
  QT_CHECK(h != NULL, "lt_dlopen(libz)");
  if (!h) { printf("  info %s\n", lt_dlerror()); lt_dlexit(); QT_END(); }
  const char *(*zlib_version)(void) = (const char *(*)(void))lt_dlsym(h, "zlibVersion");
  QT_CHECK(zlib_version != NULL, "lt_dlsym(zlibVersion)");
  if (zlib_version) {
    const char *v = zlib_version();
    QT_CHECK(v && v[0] == '1', "the resolved symbol returned a zlib version");
    printf("  info zlibVersion() = %s\n", v ? v : "(null)");
  }
  QT_CHECK(lt_dlsym(h, "no_such_symbol_here") == NULL, "missing symbol reports NULL");
  QT_CHECK(lt_dlclose(h) == 0, "lt_dlclose");
  QT_CHECK(lt_dlopen("/nonexistent/libnope.so") == NULL, "missing library reports NULL");
  QT_CHECK(lt_dlexit() == 0, "lt_dlexit");
  QT_END();
}
