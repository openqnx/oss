/* encodings (the X font encoding files, gzip-compressed): libfontenc --
 * which mkfontscale, the X server and Xft use to map font encodings to
 * Unicode -- must find its directory, read encodings.dir, resolve names and
 * aliases to files, decompress them and recode known characters: single-byte
 * tables (ISO 8859-13, CP1252 through its alias, CP866, CP1251) and the
 * large two-byte CJK ones (GBK, JIS X 0208); reverse a mapping; identify a
 * file. */
#include "common.h"
#include <X11/fonts/fontenc.h>
#include <stdlib.h>
#include <unistd.h>

static unsigned to_uni(const char *enc, unsigned code, const char **name) {
  FontEncPtr e = FontEncFind(enc, NULL);
  if (name) *name = e ? e->name : NULL;
  FontMapPtr m = e ? FontMapFind(e, FONT_ENCODING_UNICODE, -1, -1) : NULL;
  return m ? FontEncRecode(code, m) : 0xFFFFFFFFu;
}

int main(void) {
  QT_BEGIN("encodings");
  const char *dir = FontEncDirectory();
  printf("  info FontEncDirectory() = %s\n", dir ? dir : "(null)");
  QT_CHECK(dir && access(dir, R_OK) == 0, "libfontenc's encodings.dir is installed");
  if (!dir || access(dir, R_OK)) QT_END();
  FILE *f = fopen(dir, "r"); int listed = 0, present = 0; char line[512];
  char base[512]; snprintf(base, sizeof base, "%s", dir); char *sl = strrchr(base, '/'); if (sl) sl[1] = 0;
  if (f && fgets(line, sizeof line, f)) listed = atoi(line);
  int entries = 0;
  while (f && fgets(line, sizeof line, f)) {
    char nm[128], file[256];
    if (sscanf(line, "%127s %255s", nm, file) != 2) continue;
    entries++;
    char p[800]; snprintf(p, sizeof p, "%s%s", file[0] == '/' ? "" : base, file);
    present += access(p, R_OK) == 0;
  }
  if (f) fclose(f);
  printf("  info encodings.dir: %d listed, %d entries, %d files present\n", listed, entries, present);
  QT_CHECK(listed >= 60 && entries == listed && present == entries, "encodings.dir: 60+ entries, every file present");

  const char *nm = NULL;
  QT_CHECK(to_uni("iso8859-13", 0xC0, &nm) == 0x0104 && nm && !strcmp(nm, "iso8859-13"), "iso8859-13: 0xC0 -> U+0104 (A with ogonek)");
  QT_CHECK(to_uni("iso8859-13", 0xA1, NULL) == 0x201D && to_uni("iso8859-13", 0x41, NULL) == 0x41, "iso8859-13: 0xA1 -> U+201D, ASCII unchanged");
  QT_CHECK(to_uni("microsoft-ansi", 0x80, &nm) == 0x20AC && nm && !strcmp(nm, "microsoft-cp1252"), "alias microsoft-ansi -> microsoft-cp1252: 0x80 -> U+20AC (euro)");
  QT_CHECK(to_uni("ibm-cp866", 0x80, NULL) == 0x0410 && to_uni("microsoft-cp1251", 0xC0, NULL) == 0x0410, "cp866 0x80, cp1251 0xC0 -> U+0410 (Cyrillic A)");
  QT_CHECK(to_uni("koi8-r", 0xC1, NULL) == 0x0430, "koi8-r (built in): 0xC1 -> U+0430");
  QT_CHECK(to_uni("gbk-0", 0x8140, NULL) == 0x4E02 && to_uni("gbk-0", 0xB0A1, NULL) == 0x554A, "large/gbk-0: 0x8140 -> U+4E02, 0xB0A1 -> U+554A");
  QT_CHECK(to_uni("jisx0208.1990-0", 0x3021, NULL) == 0x4E9C && to_uni("jisx0208.1983-0", 0x2422, NULL) == 0x3042,
           "large/jisx0208: 0x3021 -> U+4E9C, 0x2422 -> U+3042 (hiragana a)");
  FontEncPtr e = FontEncFind("microsoft-cp1252", NULL);
  FontMapPtr m = e ? FontMapFind(e, FONT_ENCODING_UNICODE, -1, -1) : NULL;
  FontMapReversePtr rv = m ? FontMapReverse(m) : NULL;
  QT_CHECK(rv && rv->reverse(0x20AC, rv->data) == 0x80 && rv->reverse(0x2122, rv->data) == 0x99, "reverse cp1252: U+20AC -> 0x80, U+2122 -> 0x99");
  char *x = FontEncFromXLFD("-misc-fixed-medium-r-normal--13-120-75-75-c-70-iso8859-13", 58);
  QT_CHECK(x && !strcmp(x, "iso8859-13"), "encoding from an XLFD name");
  char p[800]; snprintf(p, sizeof p, "%sgbk-0.enc.gz", base);
  if (access(p, R_OK)) snprintf(p, sizeof p, "%slarge/gbk-0.enc.gz", base);
  char **ids = FontEncIdentify(p);
  QT_CHECK(ids && ids[0] && !strcmp(ids[0], "gbk-0"), "FontEncIdentify(large/gbk-0.enc.gz) -> gbk-0");
  QT_END();
}
