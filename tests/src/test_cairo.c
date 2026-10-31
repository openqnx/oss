/* cairo: rasterise known shapes into an image surface and read the pixels
 * back (fill, stroke, clip, gradients, operators), round-trip a PNG through
 * /tmp, render to PDF and SVG streams, and measure a recording surface. */
#include "common.h"
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <cairo.h>
#include <cairo-pdf.h>
#include <cairo-svg.h>

static uint32_t px(cairo_surface_t *s, int x, int y) {
  cairo_surface_flush(s);
  unsigned char *d = cairo_image_surface_get_data(s);
  return *(uint32_t *)(d + y * cairo_image_surface_get_stride(s) + x * 4);
}
struct mem { char buf[65536]; size_t n; };
static int has(const struct mem *m, const char *needle) {
  size_t k = strlen(needle);
  for (size_t i = 0; i + k <= m->n; ++i) if (memcmp(m->buf + i, needle, k) == 0) return 1;
  return 0;
}
static cairo_status_t wr(void *c, const unsigned char *d, unsigned int len) {
  struct mem *m = c;
  if (m->n + len > sizeof m->buf) len = (unsigned)(sizeof m->buf - m->n);
  memcpy(m->buf + m->n, d, len); m->n += len;
  return CAIRO_STATUS_SUCCESS;
}

int main(void) {
  QT_BEGIN("cairo");
  printf("  info cairo %s\n", cairo_version_string());
  cairo_surface_t *s = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, 64, 64);
  cairo_t *cr = cairo_create(s);
  QT_CHECK(cairo_status(cr) == CAIRO_STATUS_SUCCESS, "image surface + context");

  cairo_set_source_rgb(cr, 1, 1, 1); cairo_paint(cr);
  cairo_set_source_rgb(cr, 1, 0, 0); cairo_rectangle(cr, 8, 8, 16, 16); cairo_fill(cr);
  QT_CHECK(px(s, 10, 10) == 0xFFFF0000u && px(s, 30, 30) == 0xFFFFFFFFu, "filled rectangle: red inside, white outside");

  cairo_set_source_rgb(cr, 0, 0, 1); cairo_set_line_width(cr, 4);
  cairo_move_to(cr, 0, 40); cairo_line_to(cr, 64, 40); cairo_stroke(cr);
  QT_CHECK(px(s, 32, 40) == 0xFF0000FFu && px(s, 32, 44) == 0xFFFFFFFFu, "4 px stroke covers y 38..41 only");

  cairo_arc(cr, 48, 16, 8, 0, 2 * 3.14159265358979);
  cairo_set_source_rgba(cr, 0, 1, 0, 0.5); cairo_fill(cr);
  uint32_t c = px(s, 48, 16);
  printf("  info half-transparent green over white: %08x\n", c);
  QT_CHECK((c >> 24) == 0xFF && ((c >> 8) & 0xFF) == 0xFF && ((c >> 16) & 0xFF) >= 0x7E && ((c >> 16) & 0xFF) <= 0x81,
           "50% alpha blending (OVER)");

  cairo_save(cr);
  cairo_rectangle(cr, 0, 50, 10, 10); cairo_clip(cr);
  cairo_set_source_rgb(cr, 0, 0, 0); cairo_paint(cr);
  cairo_restore(cr);
  QT_CHECK(px(s, 5, 55) == 0xFF000000u && px(s, 15, 55) == 0xFFFFFFFFu, "clip limits paint");

  cairo_pattern_t *g = cairo_pattern_create_linear(0, 0, 64, 0);
  cairo_pattern_add_color_stop_rgb(g, 0, 0, 0, 0);
  cairo_pattern_add_color_stop_rgb(g, 1, 1, 1, 1);
  cairo_set_source(cr, g); cairo_rectangle(cr, 0, 60, 64, 4); cairo_fill(cr);
  cairo_pattern_destroy(g);
  uint32_t lo = px(s, 2, 62) & 0xFF, mid = px(s, 32, 62) & 0xFF, hi = px(s, 61, 62) & 0xFF;
  printf("  info gradient %u %u %u\n", lo, mid, hi);
  QT_CHECK(lo < 16 && mid > 112 && mid < 144 && hi > 236, "linear gradient ramps black to white");

  cairo_matrix_t mtx; cairo_matrix_init_identity(&mtx);
  cairo_matrix_translate(&mtx, 10, 20); cairo_matrix_scale(&mtx, 2, 3);
  double x = 1, y = 1; cairo_matrix_transform_point(&mtx, &x, &y);
  QT_CHECK(x == 12 && y == 23, "matrix translate+scale");

  const char *png = "/tmp/qnxport-cairo.png";
  QT_CHECK(cairo_surface_write_to_png(s, png) == CAIRO_STATUS_SUCCESS, "write PNG to /tmp");
  cairo_surface_t *rd = cairo_image_surface_create_from_png(png);
  QT_CHECK(cairo_surface_status(rd) == CAIRO_STATUS_SUCCESS && cairo_image_surface_get_width(rd) == 64 &&
           (px(rd, 10, 10) & 0xFFFFFF) == 0xFF0000u && (px(rd, 32, 40) & 0xFFFFFF) == 0x0000FFu, "PNG read back with the same pixels");
  cairo_surface_destroy(rd);
  unlink(png);
  cairo_destroy(cr); cairo_surface_destroy(s);

  static struct mem pdf, svg;
  cairo_surface_t *ps = cairo_pdf_surface_create_for_stream(wr, &pdf, 200, 100);
  cr = cairo_create(ps);
  cairo_rectangle(cr, 10, 10, 50, 50); cairo_set_source_rgb(cr, 0, 0.5, 1); cairo_fill(cr);
  cairo_show_page(cr); cairo_destroy(cr); cairo_surface_finish(ps); cairo_surface_destroy(ps);
  QT_CHECK(pdf.n > 200 && memcmp(pdf.buf, "%PDF-", 5) == 0, "PDF stream starts with %PDF-");
  QT_CHECK(has(&pdf, "%%EOF"), "PDF stream has the %%EOF trailer");
  cairo_surface_t *ss = cairo_svg_surface_create_for_stream(wr, &svg, 100, 100);
  cr = cairo_create(ss);
  cairo_arc(cr, 50, 50, 20, 0, 6.283); cairo_set_source_rgb(cr, 1, 0, 0); cairo_fill(cr);
  cairo_destroy(cr); cairo_surface_finish(ss); cairo_surface_destroy(ss);
  QT_CHECK(svg.n > 50 && has(&svg, "<svg"), "SVG stream contains <svg");

  cairo_surface_t *rec = cairo_recording_surface_create(CAIRO_CONTENT_COLOR_ALPHA, NULL);
  cr = cairo_create(rec);
  cairo_rectangle(cr, 5, 7, 20, 30); cairo_fill(cr);
  cairo_destroy(cr);
  double ex, ey, ew, eh;
  cairo_recording_surface_ink_extents(rec, &ex, &ey, &ew, &eh);
  QT_CHECK(ex == 5 && ey == 7 && ew == 20 && eh == 30, "recording surface ink extents");
  cairo_surface_destroy(rec);
  QT_END();
}
