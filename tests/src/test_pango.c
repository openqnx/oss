/* Pango: lay out and render text with DejaVu Sans found through fontconfig.
 *  - the font map resolves "DejaVu Sans" to the real face;
 *  - "MMMMMMMMMM" at 20 px, unhinted, is 10 x 1767/2048 em = 172.56 px wide
 *    (the advance of M in DejaVu Sans), one line, ten glyphs, none unknown;
 *  - word wrapping at 100 px splits a sentence into several lines;
 *  - rendering through pangocairo puts dark pixels inside the ink rectangle;
 *  - pango-segmentation marks word boundaries, pango-view renders a PNG. */
#include "common.h"
#include <stdlib.h>
#include <math.h>
#include <unistd.h>
#include <pango/pangocairo.h>

static int run_capture(const char *cmd, char *out, size_t n) {
  char full[768]; snprintf(full, sizeof full, "%s 2>&1", cmd);
  FILE *p = popen(full, "r"); if (!p) return -1;
  size_t k = fread(out, 1, n - 1, p); out[k] = 0;
  int st = pclose(p);
  printf("  info $ %s -> %d\n", cmd, st);
  return st;
}

int main(void) {
  QT_BEGIN("pango");
  printf("  info pango %s\n", pango_version_string());
  PangoFontMap *fm = pango_cairo_font_map_get_default();
  QT_CHECK(fm != NULL, "pangocairo font map");
  PangoContext *ctx = pango_font_map_create_context(fm);
  cairo_font_options_t *fo = cairo_font_options_create();
  cairo_font_options_set_hint_style(fo, CAIRO_HINT_STYLE_NONE);
  cairo_font_options_set_hint_metrics(fo, CAIRO_HINT_METRICS_OFF);
  pango_cairo_context_set_font_options(ctx, fo);
  pango_context_set_round_glyph_positions(ctx, FALSE);

  PangoFontDescription *fd = pango_font_description_from_string("DejaVu Sans");
  pango_font_description_set_absolute_size(fd, 20 * PANGO_SCALE);
  PangoFont *font = pango_font_map_load_font(fm, ctx, fd);
  QT_CHECK(font != NULL, "pango_font_map_load_font(DejaVu Sans)");
  if (font) {
    PangoFontDescription *got = pango_font_describe(font);
    const char *fam = pango_font_description_get_family(got);
    printf("  info resolved family: %s\n", fam ? fam : "(null)");
    QT_CHECK(fam && !strcmp(fam, "DejaVu Sans"), "fontconfig found DejaVu Sans");
    pango_font_description_free(got);
    g_object_unref(font);
  }
  int nfam = 0; PangoFontFamily **fams = NULL;
  pango_font_map_list_families(fm, &fams, &nfam);
  printf("  info %d font families\n", nfam);
  QT_CHECK(nfam >= 1, "font families listed");
  g_free(fams);

  PangoLayout *lay = pango_layout_new(ctx);
  pango_layout_set_font_description(lay, fd);
  pango_layout_set_text(lay, "MMMMMMMMMM", -1);
  PangoRectangle ink, log;
  pango_layout_get_extents(lay, &ink, &log);
  double w = (double)log.width / PANGO_SCALE;
  printf("  info \"MMMMMMMMMM\": logical %.2f x %.2f px, ink %.2f px wide\n", w,
         (double)log.height / PANGO_SCALE, (double)ink.width / PANGO_SCALE);
  QT_CHECK(fabs(w - 172.56) < 1.0, "width = 10 x 1767/2048 x 20 px = 172.56 px");
  QT_CHECK(pango_layout_get_line_count(lay) == 1, "one line");
  QT_CHECK(pango_layout_get_unknown_glyphs_count(lay) == 0, "no unknown glyphs");
  int glyphs = 0;
  PangoLayoutIter *it = pango_layout_get_iter(lay);
  do { PangoLayoutRun *r = pango_layout_iter_get_run_readonly(it); if (r) glyphs += r->glyphs->num_glyphs; }
  while (pango_layout_iter_next_run(it));
  pango_layout_iter_free(it);
  QT_CHECK(glyphs == 10, "ten glyphs shaped");

  pango_layout_set_text(lay, "Hello QNX", -1);
  pango_layout_get_extents(lay, &ink, &log);
  printf("  info \"Hello QNX\": %.2f px wide (unkerned advances sum to 101.45)\n", (double)log.width / PANGO_SCALE);
  QT_CHECK(fabs((double)log.width / PANGO_SCALE - 101.45) < 5.0, "\"Hello QNX\" ~101 px wide (kerning aside)");
  int idx = -1, trailing = -1;
  pango_layout_xy_to_index(lay, 0, 0, &idx, &trailing);
  QT_CHECK(idx == 0, "xy_to_index(0,0) is the first character");

  pango_layout_set_text(lay, "alpha beta gamma delta epsilon zeta", -1);
  pango_layout_set_width(lay, 100 * PANGO_SCALE);
  pango_layout_set_wrap(lay, PANGO_WRAP_WORD);
  int lines = pango_layout_get_line_count(lay);
  pango_layout_get_extents(lay, &ink, &log);
  printf("  info wrapped at 100 px: %d lines, %.1f px wide\n", lines, (double)log.width / PANGO_SCALE);
  QT_CHECK(lines >= 3, "word wrap at 100 px gives >= 3 lines");
  QT_CHECK(log.width <= 100 * PANGO_SCALE, "no line wider than 100 px");
  pango_layout_set_width(lay, -1);

  /* ---- render ---- */
  cairo_surface_t *s = cairo_image_surface_create(CAIRO_FORMAT_RGB24, 200, 40);
  cairo_t *cr = cairo_create(s);
  cairo_set_source_rgb(cr, 1, 1, 1); cairo_paint(cr);
  cairo_set_source_rgb(cr, 0, 0, 0);
  pango_layout_set_text(lay, "Hello QNX", -1);
  pango_cairo_update_context(cr, ctx);
  pango_layout_context_changed(lay);
  pango_layout_get_pixel_extents(lay, &ink, NULL);
  cairo_move_to(cr, 4, 4);
  pango_cairo_show_layout(cr, lay);
  cairo_surface_flush(s);
  unsigned char *d = cairo_image_surface_get_data(s);
  int stride = cairo_image_surface_get_stride(s), dark = 0, outside = 0;
  for (int y = 0; y < 40; y++)
    for (int x = 0; x < 200; x++) {
      unsigned px = *(unsigned *)(d + y * stride + x * 4) & 0xffffff;
      if ((px & 0xff) < 128) {
        dark++;
        if (x < 4 + ink.x - 1 || x > 4 + ink.x + ink.width + 1 || y < 4 + ink.y - 1 || y > 4 + ink.y + ink.height + 1) outside++;
      }
    }
  printf("  info rendered: %d dark pixels, %d outside the ink rect %dx%d+%d+%d\n", dark, outside, ink.width,
         ink.height, ink.x, ink.y);
  QT_CHECK(dark > 100, "text was drawn");
  QT_CHECK(outside == 0, "every drawn pixel is inside the ink rectangle");
  cairo_destroy(cr); cairo_surface_destroy(s);
  g_object_unref(lay);
  pango_font_description_free(fd);
  cairo_font_options_destroy(fo);
  g_object_unref(ctx);

  /* ---- the programs ---- */
  static char out[8192];
  int st = run_capture("pango-segmentation --kind=word --text 'Hello QNX'", out, sizeof out);
  printf("  info output: %s", out);
  QT_CHECK(st == 0 && !strncmp(out, "|Hello| |QNX|", 13), "pango-segmentation word boundaries |Hello| |QNX|");
  st = run_capture("pango-segmentation --kind=sentence --text 'One. Two.'", out, sizeof out);
  printf("  info output: %s", out);
  QT_CHECK(st == 0 && !strncmp(out, "|One. |Two.|", 12), "pango-segmentation sentence boundaries");
  char dir[64] = "/tmp/qnxport-pango.XXXXXX";
  QT_CHECK(mkdtemp(dir) != NULL, "scratch directory");
  char cmd[512];
  snprintf(cmd, sizeof cmd, "pango-view --backend=cairo -q --font='DejaVu Sans 20' --text='QNX pango' -o %s/v.png", dir);
  st = run_capture(cmd, out, sizeof out);
  if (st) printf("  info output: %s\n", out);
  snprintf(cmd, sizeof cmd, "%s/v.png", dir);
  unsigned char h[24] = {0};
  FILE *f = fopen(cmd, "rb");
  if (f) { if (fread(h, 1, 24, f) != 24) h[0] = 0; fclose(f); }
  unsigned pw = (unsigned)h[16] << 24 | h[17] << 16 | h[18] << 8 | h[19];
  unsigned ph = (unsigned)h[20] << 24 | h[21] << 16 | h[22] << 8 | h[23];
  printf("  info pango-view PNG %ux%u\n", pw, ph);
  QT_CHECK(st == 0 && !memcmp(h, "\x89PNG", 4), "pango-view wrote a PNG");
  QT_CHECK(pw > 100 && pw < 400 && ph > 20 && ph < 100, "PNG is one line of 20 pt text");
  snprintf(cmd, sizeof cmd, "rm -rf %s", dir); system(cmd);
  QT_END();
}
