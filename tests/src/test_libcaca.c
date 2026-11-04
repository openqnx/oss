/* libcaca: render text on a canvas and export it -- plain UTF-8 text with the
 * exact known lines, ANSI with colour escapes, HTML and SVG with the text in
 * them -- draw a box and a line with known characters at known cells, import
 * text back into a canvas, and dither a gradient bitmap into characters
 * (a dark-to-bright ramp gives sparse-to-dense glyphs). No terminal needed. */
#include "common.h"
#include <caca.h>
#include <stdlib.h>
#include <stdint.h>

int main(void) {
  QT_BEGIN("libcaca");
  printf("  info libcaca %s\n", caca_get_version());
  caca_canvas_t *cv = caca_create_canvas(12, 3);
  QT_CHECK(cv && caca_get_canvas_width(cv) == 12 && caca_get_canvas_height(cv) == 3, "12x3 canvas");
  caca_set_color_ansi(cv, CACA_LIGHTRED, CACA_BLACK);
  caca_put_str(cv, 1, 0, "Hello QNX");
  caca_set_color_ansi(cv, CACA_WHITE, CACA_BLUE);
  caca_draw_line(cv, 0, 2, 11, 2, '=');
  QT_CHECK(caca_get_char(cv, 1, 0) == 'H' && caca_get_char(cv, 9, 0) == 'X' && caca_get_char(cv, 0, 0) == ' ', "text at (1,0)");
  QT_CHECK(caca_get_char(cv, 0, 2) == '=' && caca_get_char(cv, 11, 2) == '=', "line of '=' along row 2");
  size_t len = 0;
  char *txt = caca_export_canvas_to_memory(cv, "utf8", &len);
  const char *want = " Hello QNX  \n            \n============\n";
  QT_CHECK(txt && len >= strlen(want), "exported as UTF-8 text");
  /* utf8 export carries colour escapes; strip ESC[...m to compare the text */
  char plain[256]; size_t o = 0;
  for (size_t i = 0; txt && i < len && o < sizeof plain - 1; i++) {
    if (txt[i] == 0x1b) { while (i < len && txt[i] != 'm') i++; continue; }
    plain[o++] = txt[i];
  }
  plain[o] = 0;
  QT_CHECK(!strcmp(plain, want), "the text lines are exactly ' Hello QNX  ', blank, '============'");
  free(txt);
  txt = caca_export_canvas_to_memory(cv, "ansi", &len);
  QT_CHECK(txt && memmem(txt, len, "\x1b[", 2) && memmem(txt, len, "Hello QNX", 9), "ANSI export: colour escapes and the text");
  free(txt);
  txt = caca_export_canvas_to_memory(cv, "html", &len);
  QT_CHECK(txt && memmem(txt, len, "<html", 5) && memmem(txt, len, "Hello", 5) && memmem(txt, len, "QNX", 3), "HTML export");
  free(txt);
  txt = caca_export_canvas_to_memory(cv, "svg", &len);
  QT_CHECK(txt && memmem(txt, len, "<svg", 4), "SVG export");
  free(txt);

  caca_canvas_t *cv2 = caca_create_canvas(0, 0);
  ssize_t r = caca_import_canvas_from_memory(cv2, "ab\ncd\n", 6, "text");
  QT_CHECK(r > 0 && caca_get_canvas_width(cv2) == 2 && caca_get_canvas_height(cv2) == 2 && caca_get_char(cv2, 1, 1) == 'd', "text import: 2x2, 'd' at (1,1)");
  caca_free_canvas(cv2);

  /* dither a horizontal grey ramp 64x8 into a 16x2 canvas */
  uint32_t bmp[64 * 8];
  for (int y = 0; y < 8; y++) for (int x = 0; x < 64; x++) { uint32_t g = x * 4; bmp[y * 64 + x] = 0xff000000u | g << 16 | g << 8 | g; }
  caca_canvas_t *dc = caca_create_canvas(16, 2);
  caca_dither_t *dt = caca_create_dither(32, 64, 8, 64 * 4, 0x00ff0000, 0x0000ff00, 0x000000ff, 0xff000000);
  QT_CHECK(dt != NULL, "dither for 32-bit ARGB");
  caca_set_dither_color(dt, "mono");
  caca_set_dither_charset(dt, "ascii");
  caca_set_dither_algorithm(dt, "none");
  caca_dither_bitmap(dc, 0, 0, 16, 2, dt, bmp);
  int left = 0, right = 0;
  for (int y = 0; y < 2; y++) {
    for (int x = 0; x < 4; x++) left += caca_get_char(dc, x, y) != ' ';
    for (int x = 12; x < 16; x++) right += caca_get_char(dc, x, y) != ' ';
  }
  txt = caca_export_canvas_to_memory(dc, "utf8", &len);
  printf("  info dithered ramp, %d/%d non-blank cells left/right\n", left, right);
  free(txt);
  QT_CHECK(left < right && right == 8, "the bright end is all glyphs, the dark end sparser");
  caca_free_dither(dt);
  caca_free_canvas(dc);
  caca_free_canvas(cv);
  QT_END();
}
