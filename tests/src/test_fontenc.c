/* libfontenc: X font encodings -- XLFD parsing, the built-in ISO 8859 / KOI8
 * tables recoding to Unicode (known code points), reverse maps, and an
 * encoding file of our own found through FONT_ENCODINGS_DIRECTORY. */
#include "common.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <X11/fonts/fontenc.h>

int main(void) {
  QT_BEGIN("fontenc");
  const char *dir = "/tmp/qnxport-fontenc";
  mkdir(dir, 0755);
  FILE *f = fopen("/tmp/qnxport-fontenc/qnx-test.enc", "w");
  if (f) {
    fputs("STARTENCODING qnx-test\nALIAS qnxtest\nSIZE 0x100\nSTARTMAPPING unicode\n"
          "0x41 0x263A\n0x42 0x2603\nENDMAPPING\nENDENCODING\n", f);
    fclose(f);
  }
  f = fopen("/tmp/qnxport-fontenc/encodings.dir", "w");
  if (f) { fputs("1\nqnx-test /tmp/qnxport-fontenc/qnx-test.enc\n", f); fclose(f); }
  setenv("FONT_ENCODINGS_DIRECTORY", "/tmp/qnxport-fontenc/encodings.dir", 1);

  const char *xlfd = "-misc-fixed-medium-r-normal--13-120-75-75-c-70-iso8859-2";
  char *e = FontEncFromXLFD(xlfd, (int)strlen(xlfd));
  QT_CHECK(e && strcmp(e, "iso8859-2") == 0, "encoding name from an XLFD");
  QT_CHECK(strcmp(FontEncDirectory(), "/tmp/qnxport-fontenc/encodings.dir") == 0, "FONT_ENCODINGS_DIRECTORY honoured");

  FontMapPtr l2 = FontEncMapFind("iso8859-2", FONT_ENCODING_UNICODE, -1, -1, NULL);
  QT_CHECK(l2 != NULL, "built-in iso8859-2 Unicode map");
  if (l2) {
    QT_CHECK(FontEncRecode(0x41, l2) == 0x41, "iso8859-2: 0x41 -> U+0041");
    QT_CHECK(FontEncRecode(0xA1, l2) == 0x0104 && FontEncRecode(0xB9, l2) == 0x0161, "iso8859-2: 0xA1 -> U+0104, 0xB9 -> U+0161");
    FontMapReversePtr r = FontMapReverse(l2);
    QT_CHECK(r && r->reverse(0x0158, r->data) == 0xD8, "reverse map: U+0158 -> 0xD8");
    if (r) FontMapReverseFree(r);
  }
  FontMapPtr koi = FontEncMapFind("koi8-r", FONT_ENCODING_UNICODE, -1, -1, NULL);
  QT_CHECK(koi && FontEncRecode(0xC1, koi) == 0x0430 && FontEncRecode(0xFF, koi) == 0x042A, "koi8-r: 0xC1 -> U+0430, 0xFF -> U+042A");
  FontMapPtr l15 = FontEncMapFind("iso8859-15", FONT_ENCODING_UNICODE, -1, -1, NULL);
  QT_CHECK(l15 && FontEncRecode(0xA4, l15) == 0x20AC, "iso8859-15: 0xA4 -> euro sign");
  FontEncPtr enc = FontEncFind("iso8859-5", NULL);
  QT_CHECK(enc && enc->size == 256 && enc->row_size == 0, "FontEncFind: iso8859-5 is a 256-entry single-byte encoding");

  char **ids = FontEncIdentify("/tmp/qnxport-fontenc/qnx-test.enc");
  QT_CHECK(ids && ids[0] && strcmp(ids[0], "qnx-test") == 0 && ids[1] && strcmp(ids[1], "qnxtest") == 0,
           "FontEncIdentify reads name and alias from an encoding file");
  if (ids) free(ids);
  FontMapPtr mine = FontEncMapFind("qnx-test", FONT_ENCODING_UNICODE, -1, -1, NULL);
  QT_CHECK(mine && FontEncRecode(0x41, mine) == 0x263A && FontEncRecode(0x42, mine) == 0x2603,
           "own encoding loaded via encodings.dir and recodes");

  unlink("/tmp/qnxport-fontenc/qnx-test.enc");
  unlink("/tmp/qnxport-fontenc/encodings.dir");
  rmdir(dir);
  QT_END();
}
