/* libtermkey: an abstract instance (no terminal) fed byte sequences, the
 * decoded keys compared with what they must be. Checks: a plain letter, a
 * UTF-8 character, Ctrl-A, Up (CSI A), Ctrl-Right (CSI 1;5C), F1 as SS3 P,
 * PageUp (CSI 5~), Alt-x as ESC x, an X10 mouse press decoded to button 1
 * at line 3 col 2, key names in vim format ("<C-Right>", "<M-x>"), the name
 * lookup "PageUp" -> TERMKEY_SYM_PAGEUP, and a lone ESC that is only
 * returned once the waiting time is forced. */
#include "common.h"
#include <termkey.h>

static TermKey *tk;
static int get(const char *bytes, TermKeyKey *k) {
  termkey_push_bytes(tk, bytes, strlen(bytes));
  return termkey_getkey(tk, k);
}
static const char *vim(TermKeyKey *k) {
  static char b[64]; termkey_strfkey(tk, b, sizeof b, k, TERMKEY_FORMAT_VIM); return b;
}

int main(void) {
  QT_BEGIN("libtermkey");
  tk = termkey_new_abstract("vt100", TERMKEY_FLAG_UTF8 | TERMKEY_FLAG_NOTERMIOS);
  QT_CHECK(tk != NULL, "termkey_new_abstract(\"vt100\", UTF8)");
  if (!tk) QT_END();
  TermKeyKey k;
  QT_CHECK(get("a", &k) == TERMKEY_RES_KEY && k.type == TERMKEY_TYPE_UNICODE && k.code.codepoint == 'a' && k.modifiers == 0, "\"a\" -> codepoint 'a'");
  QT_CHECK(get("\xc3\xa9", &k) == TERMKEY_RES_KEY && k.type == TERMKEY_TYPE_UNICODE && k.code.codepoint == 0xE9, "UTF-8 \"\\u00e9\" -> U+00E9");
  QT_CHECK(get("\x01", &k) == TERMKEY_RES_KEY && k.type == TERMKEY_TYPE_UNICODE && k.code.codepoint == 'a' &&
           k.modifiers == TERMKEY_KEYMOD_CTRL && !strcmp(vim(&k), "<C-a>"), "0x01 -> Ctrl-a (\"<C-a>\")");
  QT_CHECK(get("\x1b[A", &k) == TERMKEY_RES_KEY && k.type == TERMKEY_TYPE_KEYSYM && k.code.sym == TERMKEY_SYM_UP, "ESC[A -> Up");
  QT_CHECK(get("\x1b[1;5C", &k) == TERMKEY_RES_KEY && k.code.sym == TERMKEY_SYM_RIGHT && k.modifiers == TERMKEY_KEYMOD_CTRL &&
           !strcmp(vim(&k), "<C-Right>"), "ESC[1;5C -> Ctrl-Right (\"<C-Right>\")");
  QT_CHECK(get("\x1bOP", &k) == TERMKEY_RES_KEY && k.type == TERMKEY_TYPE_FUNCTION && k.code.number == 1, "ESC O P -> F1");
  QT_CHECK(get("\x1b[5~", &k) == TERMKEY_RES_KEY && k.type == TERMKEY_TYPE_KEYSYM && k.code.sym == TERMKEY_SYM_PAGEUP, "ESC[5~ -> PageUp");
  QT_CHECK(get("\x1bx", &k) == TERMKEY_RES_KEY && k.code.codepoint == 'x' && k.modifiers == TERMKEY_KEYMOD_ALT &&
           !strcmp(vim(&k), "<M-x>"), "ESC x -> Alt-x (\"<M-x>\")");
  /* X10 mouse: ESC [ M, button byte 32+0 (press, button 1), col 32+2, line 32+3 */
  int ok = get("\x1b[M\x20\x22\x23", &k) == TERMKEY_RES_KEY && k.type == TERMKEY_TYPE_MOUSE;
  TermKeyMouseEvent ev; int button = 0, line = 0, col = 0;
  if (ok) ok = termkey_interpret_mouse(tk, &k, &ev, &button, &line, &col) == TERMKEY_RES_KEY;
  QT_CHECK(ok && ev == TERMKEY_MOUSE_PRESS && button == 1 && line == 3 && col == 2, "X10 mouse report -> press, button 1, line 3, col 2");
  QT_CHECK(termkey_keyname2sym(tk, "PageUp") == TERMKEY_SYM_PAGEUP, "termkey_keyname2sym(\"PageUp\")");
  QT_CHECK(get("\x1b", &k) == TERMKEY_RES_AGAIN, "a lone ESC: wait for more (TERMKEY_RES_AGAIN)");
  QT_CHECK(termkey_getkey_force(tk, &k) == TERMKEY_RES_KEY && k.type == TERMKEY_TYPE_KEYSYM && k.code.sym == TERMKEY_SYM_ESCAPE,
           "forced: the lone ESC is the Escape key");
  QT_CHECK(termkey_getkey(tk, &k) == TERMKEY_RES_NONE, "buffer empty afterwards");
  termkey_destroy(tk);
  QT_END();
}
