/* ncurses: terminfo lookup and screen initialisation against a real terminal
 * description. A test run over ssh has no controlling tty, so the screen is
 * driven through newterm() on /dev/null with an explicit terminal type. */
#include "common.h"
#include <stdlib.h>
#include <stdio.h>
#include <curses.h>
#include <term.h>
int main(void) {
  QT_BEGIN("ncurses");
  printf("  info %s\n", curses_version());
  int err = 0;
  /* setupterm() reads the compiled terminfo database off the target. */
  int rc = setupterm("xterm", 1, &err);
  if (rc != OK) {
    printf("  info setupterm(\"xterm\") failed (rc=%d err=%d); "
           "no terminfo database on this target\n", rc, err);
    rc = setupterm("dumb", 1, &err);
  }
  QT_CHECK(rc == OK, "setupterm found a terminal description");
  if (rc != OK) QT_END();
  QT_CHECK(tigetnum("cols") != -2, "tigetnum(\"cols\") is a valid capability");
  QT_CHECK(tigetstr("clear") != (char *)-1, "tigetstr(\"clear\") is a valid capability");
  QT_CHECK(tigetflag("bogus_cap_name") == -1, "unknown capability is reported");

  FILE *in = fopen("/dev/null", "r"), *out = fopen("/dev/null", "w");
  QT_CHECK(in && out, "open /dev/null for the screen");
  SCREEN *scr = newterm("xterm", out, in);
  if (!scr) scr = newterm("dumb", out, in);
  QT_CHECK(scr != NULL, "newterm created a screen");
  if (scr) {
    set_term(scr);
    QT_CHECK(LINES > 0 && COLS > 0, "screen has a size");
    WINDOW *w = newwin(5, 20, 0, 0);
    QT_CHECK(w != NULL, "newwin");
    if (w) {
      QT_CHECK(mvwaddstr(w, 1, 1, "qnx ports") == OK, "mvwaddstr");
      QT_CHECK(wrefresh(w) == OK, "wrefresh");
      delwin(w);
    }
    endwin();
    delscreen(scr);
  }
  if (in) fclose(in);
  if (out) fclose(out);
  QT_END();
}
