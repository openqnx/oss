/* graphite2: library version and the face/font API against a real font if one
 * is present. Graphite tables are rare, so a plain TTF loading with no
 * Graphite table is the expected outcome and is checked as such. */
#include "common.h"
#include <graphite2/Font.h>
static const char *candidates[] = {
  "/usr/local/share/fonts/DejaVuSans.ttf",
  "/usr/share/fonts/DejaVuSans.ttf",
  "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf",
  NULL
};
int main(void) {
  QT_BEGIN("graphite2");
  int maj = 0, min = 0, bug = 0;
  gr_engine_version(&maj, &min, &bug);
  printf("  info graphite2 %d.%d.%d\n", maj, min, bug);
  QT_CHECK(maj >= 1, "engine version >= 1");
  gr_face *face = NULL;
  for (int i = 0; candidates[i] && !face; ++i)
    face = gr_make_file_face(candidates[i], gr_face_default);
  if (!face) {
    printf("  info no font with a Graphite table on this target\n");
    QT_CHECK(gr_make_file_face("/nonexistent.ttf", gr_face_default) == NULL,
             "missing file is reported as an error");
    QT_END();
  }
  QT_CHECK(gr_face_n_glyphs(face) > 0, "face reports a glyph count");
  gr_font *font = gr_make_font(32.0f, face);
  QT_CHECK(font != NULL, "gr_make_font");
  gr_font_destroy(font);
  gr_face_destroy(face);
  QT_END();
}
