/* libzip: write a zip into memory-backed file, read it back. */
#include "common.h"
#include <zip.h>
#include <stdlib.h>
#include <unistd.h>
int main(void) {
  QT_BEGIN("libzip");
  char path[] = "/tmp/qnx-libzip-XXXXXX"; close(mkstemp(path)); int err;
  zip_t *z = zip_open(path, ZIP_TRUNCATE, &err);
  QT_CHECK(z != NULL, "create archive");
  zip_source_t *s = zip_source_buffer(z, "hello qnx", 9, 0);
  QT_CHECK(zip_file_add(z, "a.txt", s, ZIP_FL_OVERWRITE) >= 0, "add entry");
  zip_set_file_compression(z, 0, ZIP_CM_ZSTD, 3);
  QT_CHECK(zip_close(z) == 0, "write (zstd)");
  z = zip_open(path, ZIP_RDONLY, &err); zip_file_t *f = zip_fopen(z, "a.txt", 0);
  char buf[16] = {0}; zip_fread(f, buf, sizeof buf - 1); zip_fclose(f); zip_close(z);
  QT_CHECK(!strcmp(buf, "hello qnx"), "read back");
  unlink(path);
  QT_END();
}
