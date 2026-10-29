#include "common.h"
#include <pango/pangocairo.h>
#include <cairo.h>
#include <hb.h>
#include <fribidi.h>
#include <unicode/uversion.h>
#include <unicode/ustring.h>
/* Exercises the whole text stack together: ICU for Unicode, FriBidi for bidi
 * ordering, HarfBuzz for shaping, cairo for rasterising and Pango for layout. */
int main(void) {
  QT_BEGIN("text-stack");

  /* --- ICU --- */
  UVersionInfo icuver; u_getVersion(icuver);
  char icubuf[U_MAX_VERSION_STRING_LENGTH]; u_versionToString(icuver, icubuf);
  QT_CHECK(icuver[0] > 0, "ICU version readable");
  printf("  info ICU %s\n", icubuf);

  /* --- FriBidi: an RTL run must come back reordered --- */
  FriBidiParType base = FRIBIDI_PAR_ON;
  FriBidiChar in[8] = {'a','b',0x05D0,0x05D1,0};  /* latin + two hebrew letters */
  FriBidiChar out[8] = {0};
  FriBidiStrIndex n = 4;
  /* fribidi_log2vis returns the maximum embedding level, not a character
   * count. A run mixing latin and hebrew must reach level >= 1. */
  FriBidiLevel maxlevel = fribidi_log2vis(in, n, &base, out, NULL, NULL, NULL);
  QT_CHECK(maxlevel >= 1, "fribidi_log2vis reports a bidi embedding level");
  QT_CHECK(out[2] == 0x05D1 && out[3] == 0x05D0, "hebrew pair reordered right-to-left");
  printf("  info FriBidi %s\n", fribidi_version_info);

  /* --- HarfBuzz --- */
  hb_buffer_t *buf = hb_buffer_create();
  QT_CHECK(hb_buffer_allocation_successful(buf), "hb_buffer_create");
  hb_buffer_add_utf8(buf, "Wavefront", -1, 0, -1);
  hb_buffer_guess_segment_properties(buf);
  QT_CHECK(hb_buffer_get_length(buf) == 9, "hb buffer holds the text");
  QT_CHECK(hb_buffer_get_direction(buf) == HB_DIRECTION_LTR, "direction guessed LTR");
  hb_buffer_destroy(buf);
  printf("  info HarfBuzz %s\n", hb_version_string());

  /* --- cairo: draw into an image surface --- */
  cairo_surface_t *surf = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, 200, 60);
  QT_CHECK(cairo_surface_status(surf) == CAIRO_STATUS_SUCCESS, "cairo surface created");
  cairo_t *cr = cairo_create(surf);
  cairo_set_source_rgb(cr, 1, 1, 1); cairo_paint(cr);
  cairo_set_source_rgb(cr, 0, 0, 0);
  cairo_rectangle(cr, 10, 10, 50, 20); cairo_fill(cr);
  QT_CHECK(cairo_status(cr) == CAIRO_STATUS_SUCCESS, "cairo drawing ops");

  /* --- Pango: lay text out through cairo and measure it --- */
  PangoLayout *layout = pango_cairo_create_layout(cr);
  QT_CHECK(layout != NULL, "pango_cairo_create_layout");
  pango_layout_set_text(layout, "Wavefront 123", -1);
  PangoFontDescription *desc = pango_font_description_from_string("Sans 12");
  pango_layout_set_font_description(layout, desc);
  int w = 0, h = 0;
  pango_layout_get_pixel_size(layout, &w, &h);
  QT_CHECK(pango_layout_get_character_count(layout) == 13, "pango counted the characters");
  printf("  info pango layout %dx%d px (0x0 means no fonts installed on target)\n", w, h);
  pango_cairo_show_layout(cr, layout);
  QT_CHECK(cairo_status(cr) == CAIRO_STATUS_SUCCESS, "pango_cairo_show_layout");

  /* --- write a real PNG out through cairo --- */
  cairo_status_t ws = cairo_surface_write_to_png(surf, "/tmp/textstack.png");
  QT_CHECK(ws == CAIRO_STATUS_SUCCESS, "cairo wrote a PNG");

  pango_font_description_free(desc);
  g_object_unref(layout);
  cairo_destroy(cr);
  cairo_surface_destroy(surf);
  printf("  info cairo %s, pango %s\n", cairo_version_string(), pango_version_string());
  QT_END();
}
