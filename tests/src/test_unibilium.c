/* unibilium: a terminfo parser. Reads the compiled terminfo database that the
 * test bundle carries, the same one ncurses uses. */
#include "common.h"
#include <stdlib.h>
#include <unibilium.h>
int main(void) {
  QT_BEGIN("unibilium");
  unibi_term *t = unibi_from_term("xterm");
  if (!t) t = unibi_from_term("dumb");
  QT_CHECK(t != NULL, "unibi_from_term found a terminal description");
  if (!t) { printf("  info no terminfo database on this target\n"); QT_END(); }
  const char *name = unibi_get_name(t);
  printf("  info terminal = %s\n", name ? name : "(unnamed)");
  QT_CHECK(name != NULL, "unibi_get_name");
  QT_CHECK(unibi_get_num(t, unibi_max_colors) != -1 ||
           unibi_get_num(t, unibi_columns) != -1, "a numeric capability is readable");
  QT_CHECK(unibi_get_bool(t, unibi_auto_right_margin) == 0 ||
           unibi_get_bool(t, unibi_auto_right_margin) == 1, "a boolean capability");
  unibi_destroy(t);
  /* The dummy terminal always exists and needs no database. */
  unibi_term *d = unibi_dummy();
  QT_CHECK(d != NULL, "unibi_dummy");
  unibi_set_num(d, unibi_max_colors, 256);
  QT_CHECK(unibi_get_num(d, unibi_max_colors) == 256, "capabilities are writable");
  unibi_destroy(d);
  /* unibi_name_str gives the long variable name, unibi_short_name_str the
   * terminfo capability code. */
  QT_CHECK(strcmp(unibi_name_str(unibi_clear_screen), "clear_screen") == 0,
           "unibi_name_str");
  QT_CHECK(strcmp(unibi_short_name_str(unibi_clear_screen), "clear") == 0,
           "unibi_short_name_str");
  QT_END();
}
