/* libgsf: structured files written and read back with known content.
 *  - OLE2 (MS compound document): a small stream, a storage holding a 10 kB
 *    stream (big-block chain), read back by name and path;
 *  - ZIP: a stored and a deflated member, read back, the deflated one
 *    smaller on disk;
 *  - the gsf program lists both archives and cats members out of them. */
#include "common.h"
#include <stdlib.h>
#include <unistd.h>
#include <gsf/gsf.h>

static char dir[64];
static char big[10000];

static int put(GsfOutfile *parent, const char *name, const void *d, size_t n, int zipdeflate) {
  GsfOutput *o = zipdeflate ? gsf_outfile_new_child_full(parent, name, FALSE, "compression-level", GSF_ZIP_DEFLATED, NULL)
                            : gsf_outfile_new_child(parent, name, FALSE);
  if (!o) return 0;
  int ok = gsf_output_write(o, n, d) && gsf_output_close(o);
  g_object_unref(o); return ok;
}
static int same(GsfInput *in, const void *d, size_t n) {
  if (!in) return 0;
  int ok = gsf_input_size(in) == (gsf_off_t)n;
  const guint8 *got = ok ? gsf_input_read(in, n, NULL) : NULL;
  ok = got && !memcmp(got, d, n);
  g_object_unref(in); return ok;
}
static void info_lines(const char *q) {
  for (int i = 0; i < 8 && *q; i++) {
    const char *e = strchr(q, '\n'); int len = e ? (int)(e - q) : (int)strlen(q);
    printf("  info   %.*s\n", len, q);
    q = e ? e + 1 : q + len;
  }
}
static int run_capture(const char *cmd, char *out, size_t n) {
  char full[768]; snprintf(full, sizeof full, "cd %s && %s 2>&1", dir, cmd);
  FILE *p = popen(full, "r"); if (!p) return -1;
  size_t k = fread(out, 1, n - 1, p); out[k] = 0;
  int st = pclose(p);
  printf("  info $ %s -> %d\n", cmd, st);
  return st;
}

int main(void) {
  QT_BEGIN("libgsf");
  gsf_init();
  strcpy(dir, "/tmp/qnxport-gsf.XXXXXX");
  QT_CHECK(mkdtemp(dir) != NULL, "scratch directory");
  for (size_t i = 0; i < sizeof big; i++) big[i] = "QNX libgsf big stream\n"[i % 22];
  const char *alpha = "hello alpha";
  char ole[128], zip[128];
  snprintf(ole, sizeof ole, "%s/t.ole", dir); snprintf(zip, sizeof zip, "%s/t.zip", dir);
  GError *err = NULL;

  /* ---- OLE2 write ---- */
  GsfOutput *out = gsf_output_stdio_new(ole, &err);
  GsfOutfile *of = out ? gsf_outfile_msole_new(out) : NULL;
  QT_CHECK(of != NULL, "gsf_outfile_msole_new");
  if (of) {
    int ok = put(of, "Alpha", alpha, strlen(alpha), 0);
    GsfOutfile *sub = GSF_OUTFILE(gsf_outfile_new_child(of, "Dir", TRUE));
    ok &= sub && put(sub, "Beta", big, sizeof big, 0);
    if (sub) { gsf_output_close(GSF_OUTPUT(sub)); g_object_unref(sub); }
    ok &= gsf_output_close(GSF_OUTPUT(of));
    QT_CHECK(ok, "wrote Alpha (11 B) and Dir/Beta (10000 B)");
    g_object_unref(of);
  }
  if (out) g_object_unref(out);
  g_clear_error(&err);
  unsigned char m[8] = {0};
  FILE *f = fopen(ole, "rb"); if (f) { if (fread(m, 1, 8, f) != 8) m[0] = 0; fclose(f); }
  QT_CHECK(!memcmp(m, "\xd0\xcf\x11\xe0\xa1\xb1\x1a\xe1", 8), "OLE2 signature D0CF11E0A1B11AE1");

  /* ---- OLE2 read ---- */
  GsfInput *in = gsf_input_stdio_new(ole, &err);
  GsfInfile *inf = in ? gsf_infile_msole_new(in, &err) : NULL;
  if (err) { printf("  info %s\n", err->message); g_clear_error(&err); }
  QT_CHECK(inf != NULL, "gsf_infile_msole_new");
  if (inf) {
    int n = gsf_infile_num_children(inf);
    printf("  info root has %d children\n", n);
    QT_CHECK(n == 2, "root has 2 children");
    QT_CHECK(same(gsf_infile_child_by_name(inf, "Alpha"), alpha, strlen(alpha)), "Alpha reads back");
    QT_CHECK(same(gsf_infile_child_by_vname(inf, "Dir", "Beta", NULL), big, sizeof big), "Dir/Beta reads back (10000 B)");
    QT_CHECK(gsf_infile_child_by_name(inf, "Nope") == NULL, "missing member -> NULL");
    g_object_unref(inf);
  }
  if (in) g_object_unref(in);

  /* ---- ZIP ---- */
  out = gsf_output_stdio_new(zip, &err);
  GsfOutfile *zf = out ? gsf_outfile_zip_new(out, &err) : NULL;
  g_clear_error(&err);
  QT_CHECK(zf != NULL, "gsf_outfile_zip_new");
  if (zf) {
    int ok = put(zf, "alpha.txt", alpha, strlen(alpha), 0) && put(zf, "big.txt", big, sizeof big, 1);
    ok &= gsf_output_close(GSF_OUTPUT(zf));
    QT_CHECK(ok, "wrote alpha.txt (stored) and big.txt (deflated)");
    g_object_unref(zf);
  }
  if (out) g_object_unref(out);
  f = fopen(zip, "rb"); long zsize = 0;
  if (f) { if (fread(m, 1, 4, f) != 4) m[0] = 0; fseek(f, 0, SEEK_END); zsize = ftell(f); fclose(f); }
  printf("  info t.zip is %ld bytes for %zu bytes of content\n", zsize, sizeof big + strlen(alpha));
  QT_CHECK(!memcmp(m, "PK\3\4", 4), "ZIP local header signature");
  QT_CHECK(zsize > 0 && zsize < 2000, "deflate compressed the repetitive 10 kB member");
  in = gsf_input_stdio_new(zip, &err);
  inf = in ? gsf_infile_zip_new(in, &err) : NULL;
  if (err) { printf("  info %s\n", err->message); g_clear_error(&err); }
  QT_CHECK(inf && gsf_infile_num_children(inf) == 2, "zip has 2 members");
  if (inf) {
    QT_CHECK(same(gsf_infile_child_by_name(inf, "alpha.txt"), alpha, strlen(alpha)), "alpha.txt reads back");
    QT_CHECK(same(gsf_infile_child_by_name(inf, "big.txt"), big, sizeof big), "big.txt inflates back to 10000 B");
    g_object_unref(inf);
  }
  if (in) g_object_unref(in);

  /* ---- the gsf program ---- */
  static char o[32768];
  int st = run_capture("gsf list t.ole", o, sizeof o);
  info_lines(o);
  QT_CHECK(st == 0 && strstr(o, "Alpha") && strstr(o, "Beta"), "gsf list t.ole shows Alpha and Beta");
  st = run_capture("gsf cat t.ole Alpha", o, sizeof o);
  QT_CHECK(st == 0 && !strcmp(o, alpha), "gsf cat t.ole Alpha = \"hello alpha\"");
  st = run_capture("gsf list t.zip", o, sizeof o);
  info_lines(o);
  QT_CHECK(st == 0 && strstr(o, "alpha.txt") && strstr(o, "big.txt") && strstr(o, "10000"),
           "gsf list t.zip shows both members and the 10000 B size");
  st = run_capture("gsf cat t.zip big.txt", o, sizeof o);
  QT_CHECK(st == 0 && strlen(o) == sizeof big && !memcmp(o, big, sizeof big), "gsf cat t.zip big.txt inflates exactly");
  char cmd[128]; snprintf(cmd, sizeof cmd, "rm -rf %s", dir); system(cmd);
  gsf_shutdown();
  QT_END();
}
