/* libraqm: lay out text with DejaVu Sans (font-dejavu, n3-gfx-py.extras) and
 * check the result -- "Hello" gives five glyphs in logical order with
 * positive advances and kerning-free cluster indices 0..4; the Hebrew word
 * "shalom" in a right-to-left paragraph comes out in visual order (first
 * glyph = last character, cluster 6 of the UTF-8 text); mixed "abc " + Hebrew
 * with automatic direction finds the LTR paragraph; the "liga" feature
 * merges "fi" into one glyph; and index/position mapping round-trips. */
#include "common.h"
#include <stdlib.h>
#include <ft2build.h>
#include FT_FREETYPE_H
#include <raqm.h>

static const char *fonts[] = { "/usr/local/share/fonts/font-dejavu/DejaVuSans.ttf",
                               "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf", NULL };

static raqm_t *lay(FT_Face face, const char *text, raqm_direction_t dir, const char *feature) {
  raqm_t *rq = raqm_create();
  if (!rq) return NULL;
  if (!raqm_set_text_utf8(rq, text, strlen(text)) || !raqm_set_freetype_face(rq, face) ||
      !raqm_set_par_direction(rq, dir) || (feature && !raqm_add_font_feature(rq, feature, -1)) || !raqm_layout(rq)) {
    raqm_destroy(rq); return NULL;
  }
  return rq;
}

int main(void) {
  QT_BEGIN("libraqm");
  int maj = 0, min = 0, mic = 0; raqm_version(&maj, &min, &mic);
  printf("  info libraqm %s (%d.%d.%d)\n", raqm_version_string(), maj, min, mic);
  QT_CHECK(raqm_version_atleast(0, 10, 0), "raqm_version_atleast(0.10)");
  FT_Library ft; FT_Face face = NULL;
  QT_CHECK(FT_Init_FreeType(&ft) == 0, "FreeType initialised");
  const char *path = NULL;
  for (int i = 0; fonts[i] && !face; i++) if (FT_New_Face(ft, fonts[i], 0, &face) == 0) path = fonts[i]; else face = NULL;
  QT_CHECK(face != NULL, "DejaVu Sans opened");
  if (!face) QT_END();
  printf("  info font %s\n", path);
  FT_Set_Char_Size(face, 0, 16 * 64, 72, 72);

  size_t n = 0;
  raqm_t *rq = lay(face, "Hello", RAQM_DIRECTION_DEFAULT, NULL);
  raqm_glyph_t *g = rq ? raqm_get_glyphs(rq, &n) : NULL;
  QT_CHECK(g && n == 5, "\"Hello\": 5 glyphs");
  if (g && n == 5) {
    int ok = 1, adv = 0;
    for (size_t i = 0; i < n; i++) { ok &= g[i].cluster == i && g[i].x_advance > 0; adv += g[i].x_advance; }
    QT_CHECK(ok, "logical order, clusters 0..4, positive advances");
    QT_CHECK(g[2].index == g[3].index && g[0].index != g[1].index, "both 'l' map to the same glyph");
    printf("  info width %d (1/64 px)\n", adv);
    QT_CHECK(raqm_get_par_resolved_direction(rq) == RAQM_DIRECTION_LTR, "resolved direction LTR");
    int last = -1, mono = 1;
    for (size_t i = 0; i < 5; i++) { size_t k = i; int x = 0, y = 0; mono &= raqm_index_to_position(rq, &k, &x, &y) && x > last; last = x; }
    QT_CHECK(mono, "raqm_index_to_position: positions increase along the text");
    size_t first = 9, end = 9;
    QT_CHECK(raqm_position_to_index(rq, 0, 0, &first) && first == 0 && raqm_position_to_index(rq, adv + 640, 0, &end) && end == 5,
             "raqm_position_to_index: x=0 -> 0, past the end -> 5");
  }
  raqm_destroy(rq);

  /* Hebrew shalom: U+05E9 U+05DC U+05D5 U+05DD, two UTF-8 bytes each */
  rq = lay(face, "\xd7\xa9\xd7\x9c\xd7\x95\xd7\x9d", RAQM_DIRECTION_RTL, NULL);
  g = rq ? raqm_get_glyphs(rq, &n) : NULL;
  QT_CHECK(g && n == 4, "Hebrew word: 4 glyphs");
  if (g && n == 4) QT_CHECK(g[0].cluster == 6 && g[3].cluster == 0, "right-to-left: visual order (clusters 6,4,2,0)");
  raqm_destroy(rq);

  rq = lay(face, "abc \xd7\xa9\xd7\x9c\xd7\x95\xd7\x9d", RAQM_DIRECTION_DEFAULT, NULL);
  g = rq ? raqm_get_glyphs(rq, &n) : NULL;
  QT_CHECK(g && n == 8 && raqm_get_par_resolved_direction(rq) == RAQM_DIRECTION_LTR, "mixed text: LTR paragraph, 8 glyphs");
  if (g && n == 8) QT_CHECK(g[0].cluster == 0 && g[4].cluster == 10 && g[7].cluster == 4, "the Hebrew run inside is reversed");
  raqm_destroy(rq);

  size_t n1 = 0, n2 = 0;
  raqm_t *a = lay(face, "fi", RAQM_DIRECTION_LTR, "-liga"), *b = lay(face, "fi", RAQM_DIRECTION_LTR, "+liga");
  if (a) raqm_get_glyphs(a, &n1);
  if (b) raqm_get_glyphs(b, &n2);
  printf("  info \"fi\": %zu glyph(s) without liga, %zu with\n", n1, n2);
  QT_CHECK(n1 == 2 && n2 == 1, "OpenType liga: \"fi\" becomes one glyph");
  raqm_destroy(a); raqm_destroy(b);
  FT_Done_Face(face); FT_Done_FreeType(ft);
  QT_END();
}
