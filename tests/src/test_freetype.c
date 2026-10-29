/* FreeType: initialise, load a real font off the target, render a glyph. */
#include "common.h"
#include <stdlib.h>
#include <ft2build.h>
#include FT_FREETYPE_H
static const char *candidates[] = {
  "/usr/local/share/fonts/DejaVuSans.ttf",
  "/usr/share/fonts/DejaVuSans.ttf",
  "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf",
  "/usr/lib/font/DejaVuSans.ttf",
  NULL
};
int main(void) {
  QT_BEGIN("freetype");
  FT_Library lib;
  QT_CHECK(FT_Init_FreeType(&lib) == 0, "FT_Init_FreeType");
  FT_Int maj, min, pat;
  FT_Library_Version(lib, &maj, &min, &pat);
  printf("  info FreeType %d.%d.%d\n", maj, min, pat);
  QT_CHECK(maj >= 2, "version >= 2");
  FT_Face face = NULL;
  const char *used = NULL;
  for (int i = 0; candidates[i]; ++i)
    if (FT_New_Face(lib, candidates[i], 0, &face) == 0) { used = candidates[i]; break; }
  if (!face) {
    /* No font on this target: the API still has to behave for a missing file. */
    printf("  info no TTF found on target; checking error path only\n");
    QT_CHECK(FT_New_Face(lib, "/nonexistent.ttf", 0, &face) != 0,
             "missing font file is reported as an error");
    FT_Done_FreeType(lib);
    QT_END();
  }
  printf("  info using %s (%s)\n", used, face->family_name ? face->family_name : "?");
  QT_CHECK(face->num_glyphs > 100, "face has a real glyph set");
  QT_CHECK(FT_Set_Pixel_Sizes(face, 0, 32) == 0, "FT_Set_Pixel_Sizes");
  FT_UInt gi = FT_Get_Char_Index(face, 'A');
  QT_CHECK(gi != 0, "glyph index for 'A'");
  QT_CHECK(FT_Load_Glyph(face, gi, FT_LOAD_DEFAULT) == 0, "FT_Load_Glyph");
  QT_CHECK(FT_Render_Glyph(face->glyph, FT_RENDER_MODE_NORMAL) == 0, "FT_Render_Glyph");
  FT_Bitmap *bm = &face->glyph->bitmap;
  QT_CHECK(bm->width > 0 && bm->rows > 0, "rasterised bitmap is non-empty");
  int ink = 0;
  for (unsigned y = 0; y < bm->rows; ++y)
    for (unsigned x = 0; x < bm->width; ++x)
      if (bm->buffer[y * (unsigned)bm->pitch + x] > 128) ++ink;
  QT_CHECK(ink > 10, "glyph has inked pixels");
  FT_Done_Face(face);
  FT_Done_FreeType(lib);
  QT_END();
}
