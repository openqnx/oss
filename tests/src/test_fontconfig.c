/* fontconfig: initialise the config, run a match, list the font set. */
#include "common.h"
#include <stdlib.h>
#include <fontconfig/fontconfig.h>
int main(void) {
  QT_BEGIN("fontconfig");
  printf("  info fontconfig %d\n", FcGetVersion());
  QT_CHECK(FcInit() == FcTrue, "FcInit");
  FcConfig *cfg = FcConfigGetCurrent();
  QT_CHECK(cfg != NULL, "FcConfigGetCurrent");
  FcStrList *dirs = FcConfigGetFontDirs(cfg);
  int ndirs = 0; FcChar8 *d;
  while ((d = FcStrListNext(dirs))) { ++ndirs; if (ndirs <= 3) printf("  info dir %s\n", d); }
  FcStrListDone(dirs);
  QT_CHECK(ndirs > 0, "at least one font directory is configured");
  FcFontSet *fs = FcConfigGetFonts(cfg, FcSetSystem);
  printf("  info %d font(s) in the system set\n", fs ? fs->nfont : 0);
  FcPattern *pat = FcNameParse((const FcChar8 *) "sans-serif");
  QT_CHECK(pat != NULL, "FcNameParse");
  FcConfigSubstitute(cfg, pat, FcMatchPattern);
  FcDefaultSubstitute(pat);
  FcResult res;
  FcPattern *m = FcFontMatch(cfg, pat, &res);
  if (m) {
    FcChar8 *file = NULL, *family = NULL;
    FcPatternGetString(m, FC_FILE, 0, &file);
    FcPatternGetString(m, FC_FAMILY, 0, &family);
    printf("  info matched %s (%s)\n", family ? (char *)family : "?",
           file ? (char *)file : "?");
    QT_CHECK(file != NULL, "match names a font file");
    FcPatternDestroy(m);
  } else {
    /* No fonts installed is a legitimate target state; the API must still work. */
    printf("  info no font matched (no fonts installed on this target)\n");
    QT_CHECK(res == FcResultNoMatch, "no-match is reported cleanly");
  }
  FcPatternDestroy(pat);
  FcFini();
  QT_END();
}
