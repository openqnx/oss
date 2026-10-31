/* Font packages (one program per package, FONT_PKG names it): fontconfig
 * scans the package's directory -- as an application font directory, so no
 * system configuration or cache is involved -- and must find its families,
 * their styles and the characters they cover; a family name must match back
 * to a file of this package; freetype then loads that file and renders a
 * glyph of a character the font is known to have. */
#include "common.h"
#include <fontconfig/fontconfig.h>
#include <ft2build.h>
#include FT_FREETYPE_H
#include <stdlib.h>
#include <unistd.h>

#ifndef FONT_PKG
#define FONT_PKG "font-dejavu"
#endif

struct spec {
  const char *pkg;
  int min_files;              /* font files in the package */
  const char *families[4];    /* families it must provide */
  unsigned int probe;         /* a character every family covers */
  int has_bold;               /* the first family has a bold face */
  int mono;                   /* the first family is monospaced */
  int color;                  /* a colour bitmap font (emoji) */
};
static const struct spec specs[] = {
  { "adwaita-fonts",       6, { "Adwaita Sans", "Adwaita Mono" },                      'A',     1, 0, 0 },
  { "font-cantarell",      1, { "Cantarell" },                                          'A',     1, 0, 0 },
  { "font-dejavu",        20, { "DejaVu Sans", "DejaVu Serif", "DejaVu Sans Mono" },    0x03A9,  1, 0, 0 },
  { "font-hack",           4, { "Hack" },                                               '0',     1, 1, 0 },
  { "font-inter",         30, { "Inter", "Inter Display" },                             'A',     1, 0, 0 },
  { "font-liberation",    12, { "Liberation Sans", "Liberation Serif", "Liberation Mono" }, 0x20AC, 1, 0, 0 },
  { "font-noto-emoji",     1, { "Noto Color Emoji" },                                   0x1F600, 0, 0, 1 },
  { "font-noto-sans-cjk",  7, { "Noto Sans CJK JP", "Noto Sans CJK SC", "Noto Sans CJK KR" }, 0x4E2D, 1, 0, 0 },
  { "font-noto-serif-cjk", 7, { "Noto Serif CJK JP", "Noto Serif CJK SC", "Noto Serif CJK KR" }, 0x4E2D, 1, 0, 0 },
};

static int in_dir(const char *file, const char *dir) {
  return file && !strncmp(file, dir, strlen(dir));
}

int main(void) {
  const struct spec *sp = NULL;
  for (size_t i = 0; i < sizeof specs / sizeof specs[0]; i++)
    if (!strcmp(specs[i].pkg, FONT_PKG)) sp = &specs[i];
  QT_BEGIN(FONT_PKG);
  if (!sp) { QT_CHECK(0, "no spec for " FONT_PKG); QT_END(); }

  char dir[512] = "";
  /* QNX_PORT_FONTS_ROOT: another tree to look in (checking the test off-target) */
  const char *roots[] = { getenv("QNX_PORT_FONTS_ROOT"), "/usr/local/share/fonts", "/data/oss/usr/local/share/fonts" };
  for (int i = 0; i < 3 && !dir[0]; i++) {
    char d[400];
    if (!roots[i]) continue;
    snprintf(d, sizeof d, "%s/%s", roots[i], sp->pkg);
    if (access(d, R_OK) == 0) snprintf(dir, sizeof dir, "%s/", d);
  }
  QT_CHECK(dir[0], "font directory share/fonts/" FONT_PKG);
  if (!dir[0]) QT_END();
  printf("  info %s\n", dir);
  printf("  info fontconfig %d.%d.%d\n", FC_MAJOR, FC_MINOR, FC_REVISION);

  /* An empty configuration plus this directory as application fonts. */
  FcConfig *cfg = FcConfigCreate();
  QT_CHECK(cfg && FcConfigAppFontAddDir(cfg, (const FcChar8 *)dir), "fontconfig scans the directory");
  FcFontSet *app = FcConfigGetFonts(cfg, FcSetApplication);
  int nfaces = app ? app->nfont : 0, nfiles = 0;
  for (int i = 0; i < nfaces; i++) {        /* distinct files among the faces */
    FcChar8 *file = NULL, *prev = NULL; int seen = 0;
    FcPatternGetString(app->fonts[i], FC_FILE, 0, &file);
    for (int j = 0; j < i && !seen; j++)
      seen = FcPatternGetString(app->fonts[j], FC_FILE, 0, &prev) == FcResultMatch && file && !strcmp((char *)prev, (char *)file);
    if (!seen && in_dir((const char *)file, dir)) nfiles++;
  }
  printf("  info %d face(s) in %d file(s)\n", nfaces, nfiles);
  char what[256];
  snprintf(what, sizeof what, "at least %d font file(s) found (%d)", sp->min_files, nfiles);
  QT_CHECK(nfiles >= sp->min_files, what);

  for (int f = 0; f < 4 && sp->families[f]; f++) {
    const char *fam = sp->families[f];
    /* FcFontList: every face of the family, with its style */
    FcPattern *pat = FcPatternCreate();
    FcPatternAddString(pat, FC_FAMILY, (const FcChar8 *)fam);
    FcObjectSet *os = FcObjectSetBuild(FC_FAMILY, FC_STYLE, FC_WEIGHT, FC_FILE, FC_CHARSET, FC_SPACING, (char *)0);
    FcFontSet *fs = FcFontList(cfg, pat, os);
    int n = fs ? fs->nfont : 0, bold = 0, covers = 0, mono = 0;
    char styles[256] = "";
    for (int i = 0; i < n; i++) {
      FcChar8 *st = NULL; int w = 0, sp_ = 0; FcCharSet *cs = NULL;
      if (FcPatternGetString(fs->fonts[i], FC_STYLE, 0, &st) == FcResultMatch && strlen(styles) < 180) {
        if (styles[0]) strcat(styles, ", ");
        strncat(styles, (const char *)st, 40);
      }
      if (FcPatternGetInteger(fs->fonts[i], FC_WEIGHT, 0, &w) == FcResultMatch && w >= FC_WEIGHT_BOLD) bold = 1;
      if (FcPatternGetCharSet(fs->fonts[i], FC_CHARSET, 0, &cs) == FcResultMatch && FcCharSetHasChar(cs, sp->probe)) covers++;
      if (FcPatternGetInteger(fs->fonts[i], FC_SPACING, 0, &sp_) == FcResultMatch && sp_ == FC_MONO) mono++;
    }
    printf("  info %s: %d face(s): %s%s\n", fam, n, styles, strlen(styles) >= 180 ? ", ..." : "");
    snprintf(what, sizeof what, "family \"%s\" listed", fam);
    QT_CHECK(n > 0, what);
    snprintf(what, sizeof what, "\"%s\" covers U+%04X (%d/%d faces)", fam, sp->probe, covers, n);
    QT_CHECK(n > 0 && covers == n, what);
    if (f == 0 && sp->has_bold) { snprintf(what, sizeof what, "\"%s\" has a bold face", fam); QT_CHECK(bold, what); }
    if (f == 0 && sp->mono) { snprintf(what, sizeof what, "\"%s\" is monospaced", fam); QT_CHECK(mono == n && n > 0, what); }
    if (fs) FcFontSetDestroy(fs);
    FcObjectSetDestroy(os);
    FcPatternDestroy(pat);
  }

  /* FcFontMatch: the first family's name, with the probe character asked
   * for, resolves to a file of this package. */
  const char *fam = sp->families[0];
  FcPattern *pat = FcPatternCreate();
  FcPatternAddString(pat, FC_FAMILY, (const FcChar8 *)fam);
  FcCharSet *want = FcCharSetCreate(); FcCharSetAddChar(want, sp->probe);
  FcPatternAddCharSet(pat, FC_CHARSET, want);
  FcConfigSubstitute(cfg, pat, FcMatchPattern);
  FcDefaultSubstitute(pat);
  FcResult res;
  FcPattern *m = FcFontMatch(cfg, pat, &res);
  FcChar8 *mfam = NULL, *mfile = NULL; int mindex = 0;
  if (m) {
    FcPatternGetString(m, FC_FAMILY, 0, &mfam);
    FcPatternGetString(m, FC_FILE, 0, &mfile);
    FcPatternGetInteger(m, FC_INDEX, 0, &mindex);
  }
  printf("  info match \"%s\" -> %s (index %d)\n", fam, mfile ? (const char *)mfile : "-", mindex);
  snprintf(what, sizeof what, "FcFontMatch(\"%s\") -> that family, a file of " FONT_PKG, fam);
  QT_CHECK(mfam && !strcmp((const char *)mfam, fam) && in_dir((const char *)mfile, dir), what);

  /* freetype: load the matched face, find and render the probe glyph. */
  FT_Library ft; FT_Face face = NULL;
  QT_CHECK(FT_Init_FreeType(&ft) == 0, "FT_Init_FreeType");
  int ok = mfile && FT_New_Face(ft, (const char *)mfile, mindex, &face) == 0;
  QT_CHECK(ok, "FT_New_Face on the matched file");
  if (ok) {
    printf("  info freetype: \"%s\" \"%s\", %ld glyphs, %s\n", face->family_name, face->style_name,
           face->num_glyphs, FT_IS_SCALABLE(face) ? "scalable" : "bitmap");
    FT_UInt gi = FT_Get_Char_Index(face, sp->probe);
    snprintf(what, sizeof what, "glyph for U+%04X (index %u)", sp->probe, gi);
    QT_CHECK(gi != 0, what);
    int err;
    if (sp->color || !FT_IS_SCALABLE(face) || face->num_fixed_sizes > 0) {
      if (face->num_fixed_sizes > 0) FT_Select_Size(face, 0);
      else FT_Set_Pixel_Sizes(face, 0, 32);
      err = FT_Load_Glyph(face, gi, FT_LOAD_COLOR | FT_LOAD_RENDER);
    } else {
      FT_Set_Pixel_Sizes(face, 0, 32);
      err = FT_Load_Glyph(face, gi, FT_LOAD_RENDER);
    }
    FT_Bitmap *bm = &face->glyph->bitmap;
    long ink = 0;
    if (!err && bm->buffer) {
      int bpp = bm->pixel_mode == FT_PIXEL_MODE_BGRA ? 4 : 1;
      for (unsigned r = 0; r < bm->rows; r++)
        for (unsigned c = 0; c < bm->width; c++) {
          const unsigned char *p = bm->buffer + r * bm->pitch + c * bpp;
          if (p[bpp - 1] > 128) ink++;      /* coverage, or alpha */
        }
    }
    snprintf(what, sizeof what, "rendered: %ux%u, mode %d, %ld inked pixel(s)", bm->width, bm->rows, bm->pixel_mode, ink);
    QT_CHECK(!err && bm->width > 0 && bm->rows > 0 && ink > 0, what);
    if (sp->color) QT_CHECK(bm->pixel_mode == FT_PIXEL_MODE_BGRA, "a colour (BGRA) glyph");
    FT_Done_Face(face);
  }
  FT_Done_FreeType(ft);
  if (m) FcPatternDestroy(m);
  FcCharSetDestroy(want);
  FcPatternDestroy(pat);
  FcConfigDestroy(cfg);
  QT_END();
}
