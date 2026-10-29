/* libarchive: write a two-entry tar.gz in memory, then read it back. */
#include "common.h"
#include <stdlib.h>
#include <archive.h>
#include <archive_entry.h>
static char blob[65536];
int main(void) {
  QT_BEGIN("libarchive");
  struct archive *w = archive_write_new();
  QT_CHECK(w != NULL, "archive_write_new");
  archive_write_add_filter_gzip(w);
  archive_write_set_format_pax_restricted(w);
  size_t used = 0;
  QT_CHECK(archive_write_open_memory(w, blob, sizeof blob, &used) == ARCHIVE_OK,
           "archive_write_open_memory");
  static const char body[] = "hello from qnx\n";
  for (int i = 0; i < 2; ++i) {
    char name[32];
    snprintf(name, sizeof name, "file%d.txt", i);
    struct archive_entry *e = archive_entry_new();
    archive_entry_set_pathname(e, name);
    archive_entry_set_size(e, (int64_t)(sizeof body - 1));
    archive_entry_set_filetype(e, AE_IFREG);
    archive_entry_set_perm(e, 0644);
    archive_write_header(w, e);
    archive_write_data(w, body, sizeof body - 1);
    archive_entry_free(e);
  }
  QT_CHECK(archive_write_close(w) == ARCHIVE_OK, "archive_write_close");
  archive_write_free(w);
  QT_CHECK(used > 0, "archive bytes produced");

  struct archive *r = archive_read_new();
  archive_read_support_filter_all(r);
  archive_read_support_format_all(r);
  QT_CHECK(archive_read_open_memory(r, blob, used) == ARCHIVE_OK,
           "archive_read_open_memory");
  int entries = 0; char got[64];
  struct archive_entry *e;
  while (archive_read_next_header(r, &e) == ARCHIVE_OK) {
    ++entries;
    ssize_t n = archive_read_data(r, got, sizeof got);
    if (n != (ssize_t)(sizeof body - 1) || memcmp(got, body, (size_t)n) != 0) entries = -100;
  }
  QT_CHECK(entries == 2, "both entries read back with correct contents");
  QT_CHECK(strcmp(archive_filter_name(r, 0), "gzip") == 0, "gzip filter detected");
  archive_read_free(r);
  QT_END();
}
