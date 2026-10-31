/* GNU FriBidi: the Unicode bidirectional algorithm -- character classes,
 * mirroring, visual reordering of mixed Hebrew/Latin/digit text in LTR and
 * RTL paragraphs (positions maps and embedding levels), and Arabic shaping
 * of the lam-alef ligature. */
#include "common.h"
#include <fribidi.h>

static int to_u32(const char *utf8, FriBidiChar *out) {
  return (int)fribidi_charset_to_unicode(FRIBIDI_CHAR_SET_UTF8, utf8, (FriBidiStrIndex)strlen(utf8), out);
}
static void to_utf8(const FriBidiChar *u, int n, char *out) {
  fribidi_unicode_to_charset(FRIBIDI_CHAR_SET_UTF8, u, n, out);
}

int main(void) {
  QT_BEGIN("fribidi");
  const char *ver = fribidi_version_info ? fribidi_version_info : "(no version)";
  printf("  info %.*s\n", (int)strcspn(ver, "\n"), ver);
  QT_CHECK(fribidi_get_bidi_type(0x05D0) == FRIBIDI_TYPE_RTL, "U+05D0 (alef) is R");
  QT_CHECK(fribidi_get_bidi_type(0x0627) == FRIBIDI_TYPE_AL, "U+0627 (Arabic alef) is AL");
  QT_CHECK(fribidi_get_bidi_type('7') == FRIBIDI_TYPE_EN && fribidi_get_bidi_type('a') == FRIBIDI_TYPE_LTR, "digits EN, Latin L");
  FriBidiChar mir = 0;
  QT_CHECK(fribidi_get_mirror_char('(', &mir) && mir == ')' && fribidi_get_mirror_char(0x00AB, &mir) && mir == 0x00BB,
           "mirroring: ( -> ), << -> >>");

  FriBidiChar in[64], vis[64]; char out[256];
  FriBidiStrIndex l2v[64], v2l[64]; FriBidiLevel lev[64];
  /* LTR paragraph: "abc " + alef bet gimel + " def" */
  int n = to_u32("abc \xD7\x90\xD7\x91\xD7\x92 def", in);
  FriBidiParType dir = FRIBIDI_PAR_ON;
  QT_CHECK(fribidi_log2vis(in, n, &dir, vis, l2v, v2l, lev) != 0, "log2vis on mixed text");
  to_utf8(vis, n, out);
  QT_CHECK(dir == FRIBIDI_PAR_LTR && strcmp(out, "abc \xD7\x92\xD7\x91\xD7\x90 def") == 0,
           "LTR paragraph: Hebrew run reversed in place");
  QT_CHECK(lev[4] == 1 && lev[0] == 0 && l2v[4] == 6 && v2l[6] == 4, "embedding levels and position maps");

  /* RTL paragraph detected from the first strong character: digits keep their order */
  n = to_u32("\xD7\x90\xD7\x91\xD7\x92 123", in);
  dir = FRIBIDI_PAR_ON;
  fribidi_log2vis(in, n, &dir, vis, l2v, v2l, lev);
  to_utf8(vis, n, out);
  QT_CHECK(dir == FRIBIDI_PAR_RTL && strcmp(out, "123 \xD7\x92\xD7\x91\xD7\x90") == 0, "RTL paragraph: \"123 gimel bet alef\"");
  QT_CHECK(lev[4] == 2 && lev[0] == 1, "numbers in RTL get level 2");

  /* Arabic lam + alef shapes into the isolated lam-alef ligature U+FEFB */
  n = to_u32("\xD9\x84\xD8\xA7", in);
  dir = FRIBIDI_PAR_RTL;
  int len = fribidi_log2vis(in, n, &dir, vis, NULL, NULL, NULL);
  int lig = 0;
  for (int i = 0; i < n; ++i) if (vis[i] == 0xFEFB) lig = 1;
  printf("  info lam-alef -> %04X %04X (max level %d)\n", (unsigned)vis[0], (unsigned)vis[1], len);
  QT_CHECK(lig, "lam + alef shaped to U+FEFB");

  FriBidiCharType types[64];
  n = to_u32("123 \xD7\xA9\xD7\x9C\xD7\x95\xD7\x9D abc", in);
  fribidi_get_bidi_types(in, n, types);
  QT_CHECK(fribidi_get_par_direction(types, n) == FRIBIDI_PAR_RTL, "paragraph direction from the first strong character (R after digits)");
  QT_END();
}
