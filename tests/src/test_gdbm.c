/* gdbm: create a database in a temp file, store, fetch, delete. */
#include "common.h"
#include <stdlib.h>
#include <unistd.h>
#include <gdbm.h>
static datum D(const char *s) { datum d; d.dptr = (char *)s; d.dsize = (int)strlen(s) + 1; return d; }
int main(void) {
  QT_BEGIN("gdbm");
  char path[] = "/tmp/qnxport-gdbm-XXXXXX";
  int fd = mkstemp(path);
  QT_CHECK(fd >= 0, "create temp file");
  if (fd >= 0) close(fd);
  unlink(path);
  GDBM_FILE db = gdbm_open(path, 0, GDBM_NEWDB, 0600, NULL);
  QT_CHECK(db != NULL, "gdbm_open");
  if (!db) { printf("  info %s\n", gdbm_strerror(gdbm_errno)); QT_END(); }
  QT_CHECK(gdbm_store(db, D("arch"), D("aarch64le"), GDBM_INSERT) == 0, "gdbm_store");
  QT_CHECK(gdbm_store(db, D("arch"), D("x86_64"), GDBM_INSERT) != 0,
           "duplicate insert is refused");
  QT_CHECK(gdbm_store(db, D("arch"), D("x86_64"), GDBM_REPLACE) == 0, "replace works");
  datum v = gdbm_fetch(db, D("arch"));
  QT_CHECK(v.dptr && strcmp(v.dptr, "x86_64") == 0, "gdbm_fetch returns the new value");
  free(v.dptr);
  QT_CHECK(gdbm_exists(db, D("arch")), "gdbm_exists");
  QT_CHECK(gdbm_delete(db, D("arch")) == 0, "gdbm_delete");
  QT_CHECK(!gdbm_exists(db, D("arch")), "key is gone after delete");
  gdbm_close(db);
  unlink(path);
  QT_END();
}
