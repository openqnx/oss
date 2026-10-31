/* SDL2_ttf: open DejaVu Sans Mono (font-dejavu package), check its face
 * metadata, that it is fixed-width (so a string's width is exactly N glyph
 * advances), glyph coverage, and render text with Solid and Blended: ink
 * present, inside the measured box, more ink for "WWW" than for "iii",
 * and UTF-8 text (accented, Greek) renders too. No display needed. */
#include "common.h"
#include <stdlib.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
static const char *fonts[] = {
  "/usr/local/share/fonts/font-dejavu/DejaVuSansMono.ttf",
  "/data/oss/usr/local/share/fonts/font-dejavu/DejaVuSansMono.ttf", NULL };
static long ink(SDL_Surface *s) {   /* ARGB8888 from Blended: count pixels with alpha */
  long n = 0;
  for (int y = 0; y < s->h; y++) for (int x = 0; x < s->w; x++) {
    Uint32 p = ((Uint32 *)((Uint8 *)s->pixels + y * s->pitch))[x];
    Uint8 r, g, b, a; SDL_GetRGBA(p, s->format, &r, &g, &b, &a);
    if (a > 64) n++;
  }
  return n;
}
int main(void) {
  QT_BEGIN("sdl2-ttf");
  const SDL_version *v = TTF_Linked_Version();
  int fm, fn, fp; TTF_GetFreeTypeVersion(&fm, &fn, &fp);
  printf("  info SDL2_ttf %d.%d.%d, FreeType %d.%d.%d\n", v->major, v->minor, v->patch, fm, fn, fp);
  QT_CHECK(TTF_Init() == 0, "TTF_Init");
  TTF_Font *f = NULL; const char *used = NULL;
  for (int i = 0; fonts[i] && !f; i++) if ((f = TTF_OpenFont(fonts[i], 24))) used = fonts[i];
  QT_CHECK(f != NULL, "TTF_OpenFont DejaVuSansMono.ttf at 24 pt");
  if (!f) { printf("  info %s\n", TTF_GetError()); QT_END(); }
  printf("  info %s\n", used);
  const char *fam = TTF_FontFaceFamilyName(f), *sty = TTF_FontFaceStyleName(f);
  printf("  info family \"%s\" style \"%s\" height %d ascent %d descent %d\n", fam ? fam : "(null)",
         sty ? sty : "(null)", TTF_FontHeight(f), TTF_FontAscent(f), TTF_FontDescent(f));
  QT_CHECK(fam && strcmp(fam, "DejaVu Sans Mono") == 0, "family name DejaVu Sans Mono");
  QT_CHECK(sty && strcmp(sty, "Book") == 0, "style name Book");
  QT_CHECK(TTF_FontFaceIsFixedWidth(f) > 0, "face is fixed width");
  QT_CHECK(TTF_FontHeight(f) > 20 && TTF_FontHeight(f) < 40 && TTF_FontAscent(f) > 0 && TTF_FontDescent(f) < 0,
           "plausible vertical metrics for 24 pt");
  int minx, maxx, miny, maxy, adv, advW;
  QT_CHECK(TTF_GlyphMetrics(f, 'M', &minx, &maxx, &miny, &maxy, &adv) == 0 && adv > 0, "glyph metrics for 'M'");
  TTF_GlyphMetrics(f, 'W', NULL, NULL, NULL, NULL, &advW);
  printf("  info advance M=%d W=%d, M box x %d..%d y %d..%d\n", adv, advW, minx, maxx, miny, maxy);
  QT_CHECK(adv == advW, "monospace: equal advances");
  int w = 0, h = 0;
  QT_CHECK(TTF_SizeText(f, "Hello QNX!", &w, &h) == 0, "TTF_SizeText");
  printf("  info \"Hello QNX!\" -> %dx%d\n", w, h);
  QT_CHECK(abs(w - 10 * adv) <= 2 && h == TTF_FontHeight(f), "width = 10 advances (+-2 px), height = font height");
  QT_CHECK(TTF_GlyphIsProvided32(f, 'A') && TTF_GlyphIsProvided32(f, 0x03A9) && !TTF_GlyphIsProvided32(f, 0x4E2D),
           "coverage: 'A' and Greek Omega yes, CJK no");

  SDL_Color white = { 255, 255, 255, 255 };
  SDL_Surface *s = TTF_RenderText_Solid(f, "Hello QNX!", white);
  QT_CHECK(s && abs(s->w - w) <= 2 && s->h == h, "Solid render has the measured size");
  if (s) {
    long n = 0;
    for (int y = 0; y < s->h; y++) for (int x = 0; x < s->w; x++) if (((Uint8 *)s->pixels)[y * s->pitch + x]) n++;
    printf("  info solid: %ld set pixels\n", n);
    QT_CHECK(n > 100 && n < s->w * s->h / 2, "Solid render has ink");
    SDL_FreeSurface(s);
  }
  SDL_Surface *a = TTF_RenderText_Blended(f, "WWW", white), *b = TTF_RenderText_Blended(f, "iii", white);
  QT_CHECK(a && b, "Blended render");
  if (a && b) {
    long ia = ink(a), ib = ink(b);
    printf("  info ink WWW=%ld iii=%ld\n", ia, ib);
    QT_CHECK(ia > ib && ib > 0, "more ink for WWW than for iii");
  }
  if (a) SDL_FreeSurface(a);
  if (b) SDL_FreeSurface(b);
  s = TTF_RenderUTF8_Blended(f, "\xc3\xa9t\xc3\xa9 \xce\xa9", white);   /* "été Ω" */
  TTF_SizeUTF8(f, "\xc3\xa9t\xc3\xa9 \xce\xa9", &w, &h);
  printf("  info UTF-8 width %d\n", w);
  QT_CHECK(s && abs(w - 5 * adv) <= 2 && ink(s) > 50, "UTF-8 text (accents, Greek) renders: 5 advances wide");
  if (s) SDL_FreeSurface(s);
  TTF_SetFontStyle(f, TTF_STYLE_BOLD);
  QT_CHECK(TTF_GetFontStyle(f) == TTF_STYLE_BOLD, "style set to bold");
  TTF_CloseFont(f);
  TTF_Quit();
  QT_END();
}
