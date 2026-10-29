/* libthai: Thai word breaking, which reads the dictionary generated at build
 * time by trietool -- so this also proves the native host tool did its job. */
#include "common.h"
#include <thai/thbrk.h>
#include <thai/thwchar.h>
#include <thai/thctype.h>
int main(void) {
  QT_BEGIN("libthai");
  /* "ภาษาไทย" (Thai language) in TIS-620: two words, ภาษา + ไทย */
  static const thchar_t text[] = { 0xC0, 0xD2, 0xC9, 0xD2, 0xE4, 0xB7, 0xC2, 0 };
  QT_CHECK(th_isthai(text[0]), "th_isthai on a Thai byte");
  QT_CHECK(!th_isthai('A'), "th_isthai on ASCII");
  QT_CHECK(!th_isupvowel(text[0]), "th_isupvowel classifies a consonant correctly");
  int pos[8];
  int n = th_brk(text, pos, 8);
  printf("  info th_brk found %d break point(s)\n", n);
  QT_CHECK(n >= 1, "th_brk found a word boundary (the dictionary loaded)");
  if (n >= 1) QT_CHECK(pos[0] > 0 && pos[0] < 7, "the break is inside the string");
  /* Break positions must be strictly increasing. */
  int ok = 1;
  for (int i = 1; i < n; ++i) if (pos[i] <= pos[i-1]) ok = 0;
  QT_CHECK(ok, "break positions are ordered");
  thchar_t out[32];
  int m = th_brk_line(text, out, sizeof out, "|");
  QT_CHECK(m > 0, "th_brk_line inserted the delimiter");
  QT_END();
}
