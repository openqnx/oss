/* libunibreak: Unicode line and word breaking (UAX #14 / #29). */
#include "common.h"
#include <linebreak.h>
#include <wordbreak.h>
int main(void) {
  QT_BEGIN("libunibreak");
  init_linebreak();
  const char *text = "QNX SDP 8.0 ports";
  size_t n = strlen(text);
  char brks[64];
  set_linebreaks_utf8((const utf8_t *)text, n, "en", brks);
  int allowed = 0, mandatory = 0;
  for (size_t i = 0; i < n; ++i) {
    if (brks[i] == LINEBREAK_ALLOWBREAK) ++allowed;
    if (brks[i] == LINEBREAK_MUSTBREAK) ++mandatory;
  }
  QT_CHECK(allowed >= 2, "line breaks are allowed at the spaces");
  /* Text that does not end in a line terminator ends INDETERMINATE, not
   * MUSTBREAK -- the caller is expected to decide what follows. */
  QT_CHECK(brks[n - 1] == LINEBREAK_INDETERMINATE,
           "text without a terminator ends indeterminate");
  QT_CHECK(mandatory == 0, "no mandatory break inside the line");

  const char *para = "one\ntwo";
  char pbrks[16];
  set_linebreaks_utf8((const utf8_t *)para, strlen(para), "en", pbrks);
  QT_CHECK(pbrks[3] == LINEBREAK_MUSTBREAK, "a newline is a mandatory break");
  /* A non-breaking space must not be a break opportunity. */
  const char *nb = "a\xC2\xA0" "b";
  char nbrks[8];
  set_linebreaks_utf8((const utf8_t *)nb, strlen(nb), "en", nbrks);
  QT_CHECK(nbrks[0] == LINEBREAK_NOBREAK, "no break before U+00A0");

  init_wordbreak();
  char wbrks[64];
  set_wordbreaks_utf8((const utf8_t *)text, n, "en", wbrks);
  int words = 0;
  for (size_t i = 0; i < n; ++i) if (wbrks[i] == WORDBREAK_BREAK) ++words;
  printf("  info %d word break(s) in \"%s\"\n", words, text);
  QT_CHECK(words >= 3, "word boundaries were found");
  QT_END();
}
