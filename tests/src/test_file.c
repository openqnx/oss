/* file / libmagic: identify known byte patterns (PNG, gzip, PDF, shell
 * script, plain text, this test's own ELF) through the libmagic API with the
 * installed magic.mgc, then the same files through the file(1) program. */
#include "common.h"
#include <magic.h>
#include <stdlib.h>
#include <unistd.h>

static const unsigned char png[] = {
  0x89, 'P', 'N', 'G', '\r', '\n', 0x1a, '\n',
  0, 0, 0, 13, 'I', 'H', 'D', 'R', 0, 0, 0, 1, 0, 0, 0, 1, 8, 2, 0, 0, 0,
  0x90, 0x77, 0x53, 0xde };
static const unsigned char gz[] = {
  0x1f, 0x8b, 0x08, 0x00, 0, 0, 0, 0, 0x00, 0x03, 0x03, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
static const char pdf[] = "%PDF-1.4\n1 0 obj\n<< >>\nendobj\ntrailer\n<< >>\n%%EOF\n";
static const char shs[] = "#!/bin/sh\necho hello\n";
static const char txt[] = "hello world, this is plain text\n";

static const char *S(const char *s) { return s ? s : "(null)"; }

static int has(const char *got, const char *want) { return got && strstr(got, want) != NULL; }

static int write_file(const char *path, const void *buf, size_t n) {
  FILE *f = fopen(path, "wb");
  if (!f) return 0;
  size_t w = fwrite(buf, 1, n, f);
  fclose(f);
  return w == n;
}

/* Run file(1) on one path and return its first output line. */
static int run_file(const char *prog, const char *opts, const char *path, char *out, size_t n) {
  char cmd[1024];
  snprintf(cmd, sizeof cmd, "%s %s '%s' 2>&1", prog, opts, path);
  FILE *p = popen(cmd, "r");
  out[0] = 0;
  if (!p) return -1;
  if (!fgets(out, (int)n, p)) out[0] = 0;
  char junk[256]; while (fgets(junk, sizeof junk, p)) {}
  size_t l = strlen(out); if (l && out[l - 1] == '\n') out[l - 1] = 0;
  return pclose(p);
}

int main(int argc, char **argv) {
  (void)argc;
  QT_BEGIN("file");
  printf("  info libmagic version %d\n", magic_version());
  QT_CHECK(magic_version() == 546, "magic_version() == 546 (file 5.46)");
  printf("  info default magic path: %s\n", S(magic_getpath(NULL, 0)));

  magic_t m = magic_open(MAGIC_NONE);
  QT_CHECK(m != NULL, "magic_open");
  if (!m) QT_END();
  int lr = magic_load(m, NULL);
  if (lr != 0) printf("  info magic_load: %s\n", S(magic_error(m)));
  QT_CHECK(lr == 0, "magic_load(default database)");

  const char *d;
  d = magic_buffer(m, png, sizeof png);   printf("  info png  -> %s\n", S(d));
  QT_CHECK(has(d, "PNG image data, 1 x 1, 8-bit/color RGB"), "PNG header identified with size and depth");
  d = magic_buffer(m, gz, sizeof gz);     printf("  info gz   -> %s\n", S(d));
  QT_CHECK(has(d, "gzip compressed data"), "gzip stream identified");
  d = magic_buffer(m, pdf, strlen(pdf));  printf("  info pdf  -> %s\n", S(d));
  QT_CHECK(has(d, "PDF document, version 1.4"), "PDF 1.4 identified");
  d = magic_buffer(m, shs, strlen(shs));  printf("  info sh   -> %s\n", S(d));
  QT_CHECK(has(d, "shell script"), "#!/bin/sh script identified");
  d = magic_buffer(m, txt, strlen(txt));  printf("  info txt  -> %s\n", S(d));
  QT_CHECK(has(d, "ASCII text"), "plain ASCII text identified");

  d = magic_file(m, argv[0]);             printf("  info self -> %s\n", S(d));
  QT_CHECK(has(d, "ELF 64-bit LSB"), "own binary: ELF 64-bit LSB");
#if defined(__x86_64__)
  QT_CHECK(has(d, "x86-64"), "own binary: x86-64");
#elif defined(__aarch64__)
  QT_CHECK(has(d, "aarch64"), "own binary: ARM aarch64");
#endif

  QT_CHECK(magic_setflags(m, MAGIC_MIME_TYPE) == 0, "magic_setflags(MAGIC_MIME_TYPE)");
  d = magic_buffer(m, png, sizeof png);
  QT_CHECK(d && strcmp(d, "image/png") == 0, "MIME type of PNG is image/png");
  d = magic_buffer(m, gz, sizeof gz);
  QT_CHECK(d && strcmp(d, "application/gzip") == 0, "MIME type of gzip is application/gzip");
  d = magic_buffer(m, txt, strlen(txt));
  QT_CHECK(d && strcmp(d, "text/plain") == 0, "MIME type of text is text/plain");
  magic_close(m);

  /* The file(1) program on files written to /tmp. */
  char dir[] = "/tmp/qnxport-file.XXXXXX";
  if (!mkdtemp(dir)) { QT_CHECK(0, "mkdtemp"); QT_END(); }
  char pp[256], pg[256], pt[256], out[512];
  snprintf(pp, sizeof pp, "%s/a.png", dir);
  snprintf(pg, sizeof pg, "%s/b.gz", dir);
  snprintf(pt, sizeof pt, "%s/c.txt", dir);
  QT_CHECK(write_file(pp, png, sizeof png) && write_file(pg, gz, sizeof gz) &&
           write_file(pt, txt, strlen(txt)), "sample files written");
  const char *prog = access("/usr/local/bin/file", X_OK) == 0 ? "/usr/local/bin/file" : "file";
  printf("  info program: %s\n", prog);
  int rc = run_file(prog, "-v", "/dev/null", out, sizeof out);
  printf("  info file -v: %s\n", out);
  QT_CHECK(rc == 0 && has(out, "file-5.46"), "file -v reports file-5.46");
  rc = run_file(prog, "-b", pp, out, sizeof out);
  printf("  info file -b a.png: %s\n", out);
  QT_CHECK(rc == 0 && has(out, "PNG image data, 1 x 1"), "file: PNG");
  rc = run_file(prog, "-b", pg, out, sizeof out);
  printf("  info file -b b.gz: %s\n", out);
  QT_CHECK(rc == 0 && has(out, "gzip compressed data"), "file: gzip");
  rc = run_file(prog, "-b --mime-type", pt, out, sizeof out);
  printf("  info file -b --mime-type c.txt: %s\n", out);
  QT_CHECK(rc == 0 && strcmp(out, "text/plain") == 0, "file --mime-type: text/plain");
  rc = run_file(prog, "", pt, out, sizeof out);
  QT_CHECK(rc == 0 && has(out, "c.txt: ASCII text"), "file <path>: \"<path>: ASCII text\"");
  unlink(pp); unlink(pg); unlink(pt); rmdir(dir);
  QT_END();
}
