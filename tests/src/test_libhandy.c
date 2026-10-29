/* libhandy: GNOME's adaptive widget library for GTK3. Widgets need a display,
 * but its GObject helper types do not -- HdyValueObject boxes a GValue and is
 * exercised here to prove the library initialises its types and links. */
#include "common.h"
#include <handy.h>
int main(void) {
  QT_BEGIN("libhandy");
  QT_CHECK(HDY_MAJOR_VERSION == 1, "built against libhandy 1.x");
  HdyValueObject *o = hdy_value_object_new_string("qnx");
  QT_CHECK(o != NULL, "hdy_value_object_new_string");
  QT_CHECK(HDY_IS_VALUE_OBJECT(o), "type registration works");
  const char *s = hdy_value_object_get_string(o);
  QT_CHECK(s && strcmp(s, "qnx") == 0, "boxed string round-trips");
  QT_CHECK(G_VALUE_HOLDS_STRING(hdy_value_object_get_value(o)), "GValue holds a string");
  g_object_unref(o);
  HdyValueObject *n = hdy_value_object_new_collect(G_TYPE_INT, 42);
  QT_CHECK(n && hdy_value_object_get_value(n) != NULL, "boxing a non-string type");
  g_object_unref(n);
  QT_END();
}
