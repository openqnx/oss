/* ATK: the accessibility type hierarchy and a concrete AtkObject. */
#include "common.h"
#include <atk/atk.h>
int main(void) {
  QT_BEGIN("atk");
  printf("  info atk %d.%d.%d\n", atk_get_major_version(), atk_get_minor_version(),
         atk_get_micro_version());
  AtkObject *obj = g_object_new(ATK_TYPE_OBJECT, NULL);
  QT_CHECK(obj != NULL, "create an AtkObject");
  QT_CHECK(ATK_IS_OBJECT(obj), "ATK_IS_OBJECT");
  atk_object_set_name(obj, "Build button");
  QT_CHECK(g_strcmp0(atk_object_get_name(obj), "Build button") == 0,
           "name round trip");
  atk_object_set_description(obj, "starts a port build");
  QT_CHECK(g_strcmp0(atk_object_get_description(obj), "starts a port build") == 0,
           "description round trip");
  atk_object_set_role(obj, ATK_ROLE_PUSH_BUTTON);
  QT_CHECK(atk_object_get_role(obj) == ATK_ROLE_PUSH_BUTTON, "role round trip");
  /* Do not assert a literal spelling here.  ATK 2.52 merged into at-spi2-core
     and 2.54 made ATK_ROLE_PUSH_BUTTON an alias of ATK_ROLE_BUTTON, so the
     name went from "push button" to "button".  The invariant that holds across
     both -- and the one worth testing -- is that the two functions are
     inverses, which also proves the role enum GType registered: without it
     initialize_role_names() bails out and every name comes back NULL. */
  const char *rname = atk_role_get_name(ATK_ROLE_PUSH_BUTTON);
  printf("  info role name = %s\n", rname ? rname : "(null)");
  QT_CHECK(rname != NULL && *rname != '\0', "atk_role_get_name");
  QT_CHECK(rname && atk_role_for_name(rname) == ATK_ROLE_PUSH_BUTTON,
           "atk_role_for_name inverts atk_role_get_name");
  AtkStateSet *ss = atk_object_ref_state_set(obj);
  QT_CHECK(ss != NULL, "atk_object_ref_state_set");
  if (ss) {
    atk_state_set_add_state(ss, ATK_STATE_ENABLED);
    QT_CHECK(atk_state_set_contains_state(ss, ATK_STATE_ENABLED), "state set add/contains");
    g_object_unref(ss);
  }
  QT_CHECK(atk_object_get_n_accessible_children(obj) == 0, "no children");
  g_object_unref(obj);
  QT_END();
}
