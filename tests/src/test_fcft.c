/* fcft: load "DejaVu Sans Mono" at 20 px through fontconfig (the font-dejavu
 * package), check its metrics (a monospace advance of 12 px: 0.602 em),
 * rasterize 'A' and 'W' into pixman images with ink in them, the same
 * advance for 'i' and 'W', precompose e + U+0301 into U+00E9, and shape a
 * text run with HarfBuzz (one glyph per character, advances adding up). */
#include "common.h"
#include <fcft/fcft.h>
#include <pixman.h>
#include <stdint.h>

static int ink(const struct fcft_glyph *g) {
  if (!g || !g->pix) return 0;
  int w = pixman_image_get_width(g->pix), h = pixman_image_get_height(g->pix), st = pixman_image_get_stride(g->pix);
  const uint8_t *d = (const uint8_t *)pixman_image_get_data(g->pix);
  int bpp = PIXMAN_FORMAT_BPP(pixman_image_get_format(g->pix));
  int n = 0;
  for (int y = 0; y < h; y++) for (int x = 0; x < w * bpp / 8; x++) if (d[y * st + x]) n++;
  return n;
}

int main(void) {
  QT_BEGIN("fcft");
  QT_CHECK(fcft_init(FCFT_LOG_COLORIZE_NEVER, false, FCFT_LOG_CLASS_WARNING), "fcft_init");
  enum fcft_capabilities cap = fcft_capabilities();
  QT_CHECK((cap & FCFT_CAPABILITY_GRAPHEME_SHAPING) && (cap & FCFT_CAPABILITY_TEXT_RUN_SHAPING), "built with grapheme and text-run shaping (HarfBuzz, utf8proc)");
  QT_CHECK(cap & FCFT_CAPABILITY_SVG, "built with SVG glyphs (nanosvg)");
  const char *names[] = { "DejaVu Sans Mono:pixelsize=20" };
  struct fcft_font *f = fcft_from_name(1, names, NULL);
  QT_CHECK(f != NULL, "fcft_from_name('DejaVu Sans Mono:pixelsize=20')");
  if (!f) { fcft_fini(); QT_END(); }
  printf("  info font %s: height %d ascent %d descent %d max advance %d\n", f->name ? f->name : "?", f->height, f->ascent, f->descent, f->max_advance.x);
  QT_CHECK(f->name && strstr(f->name, "DejaVu Sans Mono"), "fontconfig resolved it to DejaVu Sans Mono");
  QT_CHECK(f->max_advance.x == 12, "monospace advance 12 px (0.602 em at 20 px)");
  QT_CHECK(f->height >= 20 && f->height <= 26 && f->ascent + f->descent >= 20, "line height 20-26 px");
  const struct fcft_glyph *A = fcft_rasterize_char_utf32(f, 'A', FCFT_SUBPIXEL_NONE);
  QT_CHECK(A && A->width >= 9 && A->width <= 13 && A->height >= 13 && A->height <= 16, "'A' rasterized: about 11x15 px");
  QT_CHECK(ink(A) > 30, "'A' has ink in its pixman image");
  const struct fcft_glyph *i = fcft_rasterize_char_utf32(f, 'i', FCFT_SUBPIXEL_NONE);
  const struct fcft_glyph *W = fcft_rasterize_char_utf32(f, 'W', FCFT_SUBPIXEL_NONE);
  QT_CHECK(i && W && i->advance.x == 12 && W->advance.x == 12 && W->width > i->width, "'i' and 'W' advance alike, 'W' is wider ink");
  bool a, b, c;
  QT_CHECK(fcft_precompose(f, 'e', 0x0301, &a, &b, &c) == 0xE9, "precompose e + U+0301 = U+00E9");
  const uint32_t text[] = { 'Q', 'N', 'X', ' ', '8' };
  struct fcft_text_run *run = fcft_rasterize_text_run_utf32(f, 5, text, FCFT_SUBPIXEL_NONE);
  int adv = 0;
  for (size_t k = 0; run && k < run->count; k++) adv += run->glyphs[k]->advance.x;
  QT_CHECK(run && run->count == 5 && adv == 60, "text run 'QNX 8' shaped: 5 glyphs, 60 px");
  if (run) fcft_text_run_destroy(run);
  fcft_destroy(f);
  fcft_fini();
  QT_END();
}
