/* HarfBuzz: OpenType shaping with DejaVu Sans (its glyph ids and GSUB
 * ligatures were read from the font file on the build host): cmap lookups,
 * the "ffi" ligature, GPOS kerning, RTL Hebrew cluster order, Unicode
 * properties, sets, and hb-subset cutting the font down to two glyphs. */
#include "common.h"
#include <hb.h>
#include <hb-ot.h>
#include <hb-subset.h>

static const char *FONT = "/usr/local/share/fonts/font-dejavu/DejaVuSans.ttf";

static unsigned shape(hb_font_t *font, const char *text, const char *features, hb_glyph_info_t **gi,
                      hb_glyph_position_t **gp, hb_buffer_t **keep) {
  hb_buffer_t *b = hb_buffer_create();
  hb_buffer_add_utf8(b, text, -1, 0, -1);
  hb_buffer_guess_segment_properties(b);
  hb_feature_t f[4]; unsigned nf = 0;
  if (features) {
    const char *p = features;
    while (*p && nf < 4) {
      const char *e = strchr(p, ','); size_t l = e ? (size_t)(e - p) : strlen(p);
      if (hb_feature_from_string(p, (int)l, &f[nf])) ++nf;
      p += l + (e ? 1 : 0);
    }
  }
  hb_shape(font, b, f, nf);
  unsigned n = 0;
  *gi = hb_buffer_get_glyph_infos(b, &n);
  *gp = hb_buffer_get_glyph_positions(b, &n);
  *keep = b;
  return n;
}

int main(void) {
  QT_BEGIN("harfbuzz");
  printf("  info HarfBuzz %s\n", hb_version_string());

  /* ---- Unicode and sets: no font needed ---- */
  hb_unicode_funcs_t *uf = hb_unicode_funcs_get_default();
  QT_CHECK(hb_unicode_general_category(uf, 'a') == HB_UNICODE_GENERAL_CATEGORY_LOWERCASE_LETTER &&
           hb_unicode_general_category(uf, '7') == HB_UNICODE_GENERAL_CATEGORY_DECIMAL_NUMBER, "general categories");
  QT_CHECK(hb_unicode_mirroring(uf, '[') == ']' && hb_unicode_script(uf, 0x05D0) == HB_SCRIPT_HEBREW &&
           hb_unicode_script(uf, 0x0E01) == HB_SCRIPT_THAI, "mirroring and scripts");
  hb_set_t *s = hb_set_create();
  hb_set_add_range(s, 10, 20); hb_set_del(s, 15);
  QT_CHECK(hb_set_get_population(s) == 10 && !hb_set_has(s, 15) && hb_set_get_max(s) == 20, "hb_set range/delete");
  hb_set_destroy(s);
  QT_CHECK(hb_language_from_string("en-US", -1) == hb_language_from_string("EN-us", -1), "language tags normalised");

  hb_blob_t *blob = hb_blob_create_from_file_or_fail(FONT);
  QT_CHECK(blob != NULL, "DejaVuSans.ttf loaded");
  if (!blob) { printf("  info %s missing\n", FONT); QT_END(); }
  hb_face_t *face = hb_face_create(blob, 0);
  hb_font_t *font = hb_font_create(face);
  printf("  info %u glyphs, upem %u\n", hb_face_get_glyph_count(face), hb_face_get_upem(face));
  QT_CHECK(hb_face_get_glyph_count(face) == 6253 && hb_face_get_upem(face) == 2048, "glyph count 6253, 2048 units/em");
  hb_codepoint_t g = 0;
  QT_CHECK(hb_font_get_nominal_glyph(font, 'A', &g) && g == 36 && hb_font_get_nominal_glyph(font, 'V', &g) && g == 57,
           "cmap: A -> 36, V -> 57");
  QT_CHECK(!hb_font_get_nominal_glyph(font, 0xE123, &g), "unmapped private-use code point");
  QT_CHECK(hb_ot_layout_has_substitution(face) && hb_ot_layout_has_positioning(face), "GSUB and GPOS present");

  hb_glyph_info_t *gi; hb_glyph_position_t *gp; hb_buffer_t *b;
  unsigned n = shape(font, "office", NULL, &gi, &gp, &b);
  printf("  info \"office\" -> %u glyphs:", n);
  for (unsigned i = 0; i < n; ++i) printf(" %u@%u", gi[i].codepoint, gi[i].cluster);
  printf("\n");
  QT_CHECK(n == 4 && gi[0].codepoint == 82 && gi[1].codepoint == 5044 && gi[1].cluster == 1 && gi[2].cluster == 4,
           "\"office\": ffi ligature (glyph 5044) spanning clusters 1-3");
  QT_CHECK(hb_buffer_get_direction(b) == HB_DIRECTION_LTR && hb_buffer_get_script(b) == HB_SCRIPT_LATIN, "LTR Latin guessed");
  hb_buffer_destroy(b);
  n = shape(font, "office", "-liga", &gi, &gp, &b);
  QT_CHECK(n == 6 && gi[1].codepoint == 73 && gi[2].codepoint == 73, "\"office\" with -liga keeps f f i");
  hb_buffer_destroy(b);

  n = shape(font, "AV", NULL, &gi, &gp, &b);
  hb_position_t kerned = n == 2 ? gp[0].x_advance : 0;
  hb_buffer_destroy(b);
  n = shape(font, "AV", "-kern", &gi, &gp, &b);
  hb_position_t plain = n == 2 ? gp[0].x_advance : 0;
  hb_buffer_destroy(b);
  printf("  info A advance %d kerned, %d unkerned (font units)\n", kerned, plain);
  QT_CHECK(plain == hb_font_get_glyph_h_advance(font, 36) && kerned < plain, "GPOS kerning tightens A-V");

  n = shape(font, "\xD7\xA9\xD7\x9C\xD7\x95\xD7\x9D", NULL, &gi, &gp, &b);   /* shalom */
  QT_CHECK(n == 4 && hb_buffer_get_direction(b) == HB_DIRECTION_RTL && gi[0].cluster == 6 && gi[3].cluster == 0,
           "Hebrew shaped RTL, clusters in visual order");
  hb_buffer_destroy(b);

  hb_subset_input_t *in = hb_subset_input_create_or_fail();
  hb_set_add(hb_subset_input_unicode_set(in), 'A');
  hb_set_add(hb_subset_input_unicode_set(in), 'V');
  hb_face_t *sub = in ? hb_subset_or_fail(face, in) : NULL;
  QT_CHECK(sub != NULL, "hb-subset produced a face");
  if (sub) {
    printf("  info subset has %u glyphs\n", hb_face_get_glyph_count(sub));
    hb_font_t *sf = hb_font_create(sub);
    QT_CHECK(hb_face_get_glyph_count(sub) == 3 && hb_font_get_nominal_glyph(sf, 'V', &g) && g > 0 &&
             !hb_font_get_nominal_glyph(sf, 'B', &g), "subset: .notdef + A + V, B dropped");
    hb_font_destroy(sf);
    hb_face_destroy(sub);
  }
  hb_subset_input_destroy(in);
  hb_font_destroy(font); hb_face_destroy(face); hb_blob_destroy(blob);
  QT_END();
}
