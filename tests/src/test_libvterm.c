/* libvterm: a 25x80 terminal fed escape sequences, read back cell by cell.
 * Checks: plain text lands at the cursor, SGR bold + red (index 1) on one
 * cell and reset after it, CUP moves the cursor (ESC[2;5H -> row 1 col 4),
 * a UTF-8 "é" is one cell and a CJK "中" is two, ED clears, the screen text
 * of a row, the cursor position the state reports, the keyboard encoder
 * (Up -> ESC[A, Ctrl-A -> 0x01, Up with application cursor keys -> ESC OA),
 * and the reply to a Primary Device Attributes query (ESC[c). */
#include "common.h"
#include <vterm.h>

static VTerm *vt; static VTermScreen *scr;
static void feed(const char *s) { vterm_input_write(vt, s, strlen(s)); }
static VTermScreenCell cell(int r, int c) {
  VTermScreenCell x; VTermPos p = { r, c }; memset(&x, 0, sizeof x); vterm_screen_get_cell(scr, p, &x); return x;
}
static int out(char *buf, size_t n) { size_t k = vterm_output_read(vt, buf, n - 1); buf[k] = 0; return (int)k; }

int main(void) {
  QT_BEGIN("libvterm");
  vt = vterm_new(25, 80);
  QT_CHECK(vt != NULL, "vterm_new(25, 80)");
  vterm_set_utf8(vt, 1);
  scr = vterm_obtain_screen(vt);
  vterm_screen_reset(scr, 1);
  int rows = 0, cols = 0; vterm_get_size(vt, &rows, &cols);
  QT_CHECK(rows == 25 && cols == 80, "vterm_get_size: 25x80");

  feed("Hello\x1b[1;31mR\x1b[0mz");
  VTermScreenCell h = cell(0, 0), r = cell(0, 5), z = cell(0, 6);
  QT_CHECK(h.chars[0] == 'H' && cell(0, 4).chars[0] == 'o', "text at row 0: \"Hello\"");
  QT_CHECK(r.chars[0] == 'R' && r.attrs.bold && VTERM_COLOR_IS_INDEXED(&r.fg) && r.fg.indexed.idx == 1,
           "SGR 1;31: bold, red (palette index 1)");
  QT_CHECK(z.chars[0] == 'z' && !z.attrs.bold && VTERM_COLOR_IS_DEFAULT_FG(&z.fg), "SGR 0 resets the attributes");

  feed("\x1b[2;5HX");
  VTermPos cp; vterm_state_get_cursorpos(vterm_obtain_state(vt), &cp);
  QT_CHECK(cell(1, 4).chars[0] == 'X' && cp.row == 1 && cp.col == 5, "CUP ESC[2;5H: X at row 1 col 4, cursor after it");

  feed("\x1b[3;1H\xc3\xa9\xe4\xb8\xad!");
  VTermScreenCell e = cell(2, 0), w = cell(2, 1), bang = cell(2, 3);
  QT_CHECK(e.chars[0] == 0xE9 && e.width == 1, "UTF-8 \"\\u00e9\" is one cell");
  QT_CHECK(w.chars[0] == 0x4E2D && w.width == 2 && bang.chars[0] == '!', "CJK \"\\u4e2d\" is two cells wide");

  char text[128]; VTermRect row0 = { .start_row = 0, .end_row = 1, .start_col = 0, .end_col = 80 };
  size_t n = vterm_screen_get_text(scr, text, sizeof text - 1, row0); text[n] = 0;
  QT_CHECK(!strcmp(text, "HelloRz"), "vterm_screen_get_text of row 0: \"HelloRz\"");

  feed("\x1b[2J");
  QT_CHECK(cell(0, 0).chars[0] == 0 && cell(1, 4).chars[0] == 0, "ED 2 (ESC[2J) clears the screen");

  char buf[64];
  out(buf, sizeof buf);   /* drain */
  vterm_keyboard_key(vt, VTERM_KEY_UP, VTERM_MOD_NONE);
  out(buf, sizeof buf);
  QT_CHECK(!strcmp(buf, "\x1b[A"), "keyboard: Up -> ESC [ A");
  vterm_keyboard_unichar(vt, 'a', VTERM_MOD_CTRL);
  out(buf, sizeof buf);
  QT_CHECK(!strcmp(buf, "\x01"), "keyboard: Ctrl-A -> 0x01");
  feed("\x1b[?1h");      /* DECCKM: application cursor keys */
  vterm_keyboard_key(vt, VTERM_KEY_UP, VTERM_MOD_NONE);
  out(buf, sizeof buf);
  QT_CHECK(!strcmp(buf, "\x1bOA"), "keyboard with DECCKM: Up -> ESC O A");
  feed("\x1b[c");
  int k = out(buf, sizeof buf);
  QT_CHECK(k > 4 && !strncmp(buf, "\x1b[?", 3) && buf[k - 1] == 'c', "DA query ESC[c answered with ESC[?...c");
  printf("  info DA reply: ESC%s\n", buf + 1);
  vterm_free(vt);
  QT_END();
}
