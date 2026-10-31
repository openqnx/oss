/* libiconv: convert known text UTF-8 <-> ISO-8859-1 <-> UTF-16BE through the
 * API and check the exact bytes, reject an unconvertible character (EILSEQ),
 * transliterate it with //TRANSLIT, then run the iconv(1) program on a file
 * for the same round trip. */
#include "common.h"
#include <iconv.h>
#include <localcharset.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>

/* "Grüße, café" */
static const char utf8[] = "Gr\xc3\xbc\xc3\x9f" "e, caf\xc3\xa9";
static const char latin1[] = "Gr\xfc\xdf" "e, caf\xe9";

static size_t conv(const char *to, const char *from, const char *in, size_t inlen,
                   char *out, size_t outsz, int *err) {
  *err = 0;
  iconv_t cd = iconv_open(to, from);
  if (cd == (iconv_t)-1) { *err = errno; return (size_t)-1; }
  char *ip = (char *)in, *op = out;
  size_t il = inlen, ol = outsz;
  size_t r = iconv(cd, &ip, &il, &op, &ol);
  if (r == (size_t)-1) *err = errno;
  else iconv(cd, NULL, NULL, &op, &ol);  /* flush */
  iconv_close(cd);
  return r == (size_t)-1 ? (size_t)-1 : (size_t)(op - out);
}

static void hex(const char *tag, const char *b, size_t n) {
  printf("  info %s:", tag);
  for (size_t i = 0; i < n && i < 40; i++) printf(" %02x", (unsigned char)b[i]);
  printf("\n");
}

int main(void) {
  QT_BEGIN("libiconv");
  printf("  info _libiconv_version 0x%x\n", _libiconv_version);
  QT_CHECK(_libiconv_version == 0x0112, "_libiconv_version is 1.18");
  const char *cs = locale_charset();
  printf("  info locale_charset() = %s\n", cs ? cs : "(null)");
  QT_CHECK(cs != NULL && cs[0] != 0, "locale_charset() returns a name");

  char out[256], back[256]; int err; size_t n, m;
  n = conv("ISO-8859-1", "UTF-8", utf8, strlen(utf8), out, sizeof out, &err);
  hex("UTF-8 -> ISO-8859-1", out, n == (size_t)-1 ? 0 : n);
  QT_CHECK(n == strlen(latin1) && memcmp(out, latin1, n) == 0, "UTF-8 -> ISO-8859-1: exact bytes");
  m = conv("UTF-8", "ISO-8859-1", out, n == (size_t)-1 ? 0 : n, back, sizeof back, &err);
  QT_CHECK(m == strlen(utf8) && memcmp(back, utf8, m) == 0, "ISO-8859-1 -> UTF-8: round trip identical");

  n = conv("UTF-16BE", "UTF-8", "A\xc3\xa9\xe2\x82\xac", 6, out, sizeof out, &err);
  hex("UTF-8 -> UTF-16BE", out, n == (size_t)-1 ? 0 : n);
  QT_CHECK(n == 6 && memcmp(out, "\x00" "A" "\x00\xe9\x20\xac", 6) == 0, "UTF-8 -> UTF-16BE: A, e-acute, euro");
  n = conv("UTF-32LE", "UTF-16BE", "\xd8\x3d\xde\x00", 4, out, sizeof out, &err);
  QT_CHECK(n == 4 && memcmp(out, "\x00\xf6\x01\x00", 4) == 0, "UTF-16 surrogate pair -> U+1F600 (UTF-32LE)");

  n = conv("ISO-8859-1", "UTF-8", "price 5\xe2\x82\xac", 10, out, sizeof out, &err);
  QT_CHECK(n == (size_t)-1 && err == EILSEQ, "euro sign to ISO-8859-1 fails with EILSEQ");
  n = conv("ISO-8859-1//TRANSLIT", "UTF-8", "price 5\xe2\x82\xac", 10, out, sizeof out, &err);
  if (n != (size_t)-1) printf("  info //TRANSLIT: \"%.*s\"\n", (int)n, out);
  QT_CHECK(n == 10 && memcmp(out, "price 5EUR", 10) == 0, "//TRANSLIT turns the euro sign into EUR");
  n = conv("ASCII//TRANSLIT", "UTF-8", "caf\xc3\xa9", 5, out, sizeof out, &err);
  QT_CHECK(n != (size_t)-1 && n >= 4 && memcmp(out, "caf", 3) == 0, "ASCII//TRANSLIT of cafe-acute");
  n = conv("UTF-8", "CP1252", "\x80\x93", 2, out, sizeof out, &err);
  QT_CHECK(n == 6 && memcmp(out, "\xe2\x82\xac\xe2\x80\x9c", 6) == 0, "CP1252 0x80 0x93 -> euro, left quote");
  n = conv("UTF-8", "KOI8-R", "\xf0\xd2\xc9\xd7\xc5\xd4", 6, out, sizeof out, &err);
  QT_CHECK(n == 12 && memcmp(out, "\xd0\x9f\xd1\x80\xd0\xb8\xd0\xb2\xd0\xb5\xd1\x82", 12) == 0,
           "KOI8-R -> UTF-8 (Cyrillic word)");
  iconv_t bad = iconv_open("NO-SUCH-CHARSET", "UTF-8");
  QT_CHECK(bad == (iconv_t)-1 && errno == EINVAL, "unknown charset: EINVAL");

  /* The iconv(1) program. */
  char dir[] = "/tmp/qnxport-libiconv.XXXXXX";
  if (!mkdtemp(dir)) { QT_CHECK(0, "mkdtemp"); QT_END(); }
  char fu[256], fl[256], fb[256], cmd[1024];
  snprintf(fu, sizeof fu, "%s/u.txt", dir); snprintf(fl, sizeof fl, "%s/l.txt", dir);
  snprintf(fb, sizeof fb, "%s/b.txt", dir);
  FILE *f = fopen(fu, "wb"); if (f) { fputs(utf8, f); fclose(f); }
  const char *prog = access("/usr/local/bin/iconv", X_OK) == 0 ? "/usr/local/bin/iconv" : "iconv";
  snprintf(cmd, sizeof cmd, "%s -f UTF-8 -t ISO-8859-1 '%s' > '%s'", prog, fu, fl);
  int rc = system(cmd);
  char buf[256] = {0}; size_t got = 0;
  f = fopen(fl, "rb"); if (f) { got = fread(buf, 1, sizeof buf, f); fclose(f); }
  hex("iconv(1) output", buf, got);
  QT_CHECK(rc == 0 && got == strlen(latin1) && memcmp(buf, latin1, got) == 0, "iconv -f UTF-8 -t ISO-8859-1 file");
  snprintf(cmd, sizeof cmd, "%s -f ISO-8859-1 -t UTF-8 < '%s' > '%s'", prog, fl, fb);
  rc = system(cmd);
  got = 0; f = fopen(fb, "rb"); if (f) { got = fread(buf, 1, sizeof buf, f); fclose(f); }
  QT_CHECK(rc == 0 && got == strlen(utf8) && memcmp(buf, utf8, got) == 0, "iconv back to UTF-8 (stdin) identical");
  snprintf(cmd, sizeof cmd, "%s -l | grep -c -i 'ISO-8859-1' > '%s'", prog, fb);
  rc = system(cmd);
  QT_CHECK(rc == 0, "iconv -l lists ISO-8859-1");
  unlink(fu); unlink(fl); unlink(fb); rmdir(dir);
  QT_END();
}
