/* at-spi2-core: the ATK interfaces it ships (objects, roles, states,
 * relations) and libatspi's client-side value types (role names, state sets
 * and their algebra), with known answers. Nothing here needs the
 * accessibility bus, which a vanilla target does not run. */
#include "common.h"
#include <atk/atk.h>
#include <atspi/atspi.h>

int main(void) {
  QT_BEGIN("at-spi2-core");
  printf("  info ATK %u.%u.%u, AT-SPI %u.%u.%u\n", atk_get_major_version(), atk_get_minor_version(),
         atk_get_micro_version(), ATSPI_MAJOR_VERSION, ATSPI_MINOR_VERSION, ATSPI_MICRO_VERSION);

  /* ---- ATK ---- */
  /* Since ATK 2.46 PUSH_BUTTON is an alias of BUTTON, named "button". */
  QT_CHECK(strcmp(atk_role_get_name(ATK_ROLE_BUTTON), "button") == 0 && ATK_ROLE_PUSH_BUTTON == ATK_ROLE_BUTTON,
           "atk_role_get_name(BUTTON) (PUSH_BUTTON is its alias)");
  QT_CHECK(atk_role_for_name("check box") == ATK_ROLE_CHECK_BOX, "atk_role_for_name(\"check box\")");
  AtkRole custom = atk_role_register("qnx-gauge");
  QT_CHECK(custom >= ATK_ROLE_LAST_DEFINED && strcmp(atk_role_get_name(custom), "qnx-gauge") == 0, "register a custom role");

  AtkObject *btn = g_object_new(ATK_TYPE_OBJECT, NULL);
  AtkObject *lbl = g_object_new(ATK_TYPE_OBJECT, NULL);
  atk_object_set_name(btn, "OK");
  atk_object_set_description(btn, "Confirm the dialog");
  atk_object_set_role(btn, ATK_ROLE_PUSH_BUTTON);
  atk_object_set_name(lbl, "Proceed?");
  QT_CHECK(strcmp(atk_object_get_name(btn), "OK") == 0 && atk_object_get_role(btn) == ATK_ROLE_PUSH_BUTTON,
           "AtkObject name and role");
  QT_CHECK(strcmp(atk_object_get_description(btn), "Confirm the dialog") == 0, "AtkObject description");
  QT_CHECK(atk_object_add_relationship(btn, ATK_RELATION_LABELLED_BY, lbl), "add LABELLED_BY relation");
  AtkRelationSet *rs = atk_object_ref_relation_set(btn);
  AtkRelation *rel = atk_relation_set_get_relation_by_type(rs, ATK_RELATION_LABELLED_BY);
  GPtrArray *targets = rel ? atk_relation_get_target(rel) : NULL;
  QT_CHECK(targets && targets->len == 1 && g_ptr_array_index(targets, 0) == lbl, "relation target is the label");
  QT_CHECK(strcmp(atk_relation_type_get_name(ATK_RELATION_LABELLED_BY), "labelled-by") == 0, "relation type name");
  g_object_unref(rs);

  AtkStateSet *a = atk_state_set_new(), *b = atk_state_set_new();
  AtkStateType sa[] = {ATK_STATE_ENABLED, ATK_STATE_FOCUSABLE, ATK_STATE_VISIBLE};
  atk_state_set_add_states(a, sa, 3);
  atk_state_set_add_state(b, ATK_STATE_VISIBLE);
  atk_state_set_add_state(b, ATK_STATE_CHECKED);
  AtkStateSet *and_ = atk_state_set_and_sets(a, b), *xor_ = atk_state_set_xor_sets(a, b);
  QT_CHECK(atk_state_set_contains_state(and_, ATK_STATE_VISIBLE) && !atk_state_set_contains_state(and_, ATK_STATE_ENABLED),
           "AtkStateSet AND");
  QT_CHECK(atk_state_set_contains_state(xor_, ATK_STATE_CHECKED) && atk_state_set_contains_state(xor_, ATK_STATE_FOCUSABLE) &&
           !atk_state_set_contains_state(xor_, ATK_STATE_VISIBLE), "AtkStateSet XOR");
  QT_CHECK(strcmp(atk_state_type_get_name(ATK_STATE_FOCUSABLE), "focusable") == 0, "state type name");
  g_object_unref(a); g_object_unref(b); g_object_unref(and_); g_object_unref(xor_);
  g_object_unref(btn); g_object_unref(lbl);

  /* ---- libatspi ---- */
  gchar *rn = atspi_role_get_name(ATSPI_ROLE_BUTTON);
  QT_CHECK(rn && strcmp(rn, "button") == 0, "atspi_role_get_name(BUTTON)");
  g_free(rn);
  rn = atspi_role_get_name(ATSPI_ROLE_TABLE_CELL);
  QT_CHECK(rn && strcmp(rn, "table cell") == 0, "atspi_role_get_name(TABLE_CELL)");
  g_free(rn);

  GArray *arr = g_array_new(FALSE, FALSE, sizeof(AtspiStateType));
  AtspiStateType st[] = {ATSPI_STATE_ENABLED, ATSPI_STATE_SHOWING};
  g_array_append_vals(arr, st, 2);
  AtspiStateSet *s1 = atspi_state_set_new(arr);
  g_array_free(arr, TRUE);
  QT_CHECK(atspi_state_set_contains(s1, ATSPI_STATE_SHOWING) && !atspi_state_set_contains(s1, ATSPI_STATE_FOCUSED),
           "AtspiStateSet from an array");
  atspi_state_set_add(s1, ATSPI_STATE_FOCUSED);
  atspi_state_set_set_by_name(s1, "enabled", FALSE);
  QT_CHECK(atspi_state_set_contains(s1, ATSPI_STATE_FOCUSED) && !atspi_state_set_contains(s1, ATSPI_STATE_ENABLED),
           "add + set_by_name(\"enabled\", FALSE)");
  GArray *none = g_array_new(FALSE, FALSE, sizeof(AtspiStateType));
  AtspiStateSet *s2 = atspi_state_set_new(none);
  g_array_free(none, TRUE);
  QT_CHECK(atspi_state_set_is_empty(s2), "empty state set");
  atspi_state_set_add(s2, ATSPI_STATE_SHOWING);
  atspi_state_set_add(s2, ATSPI_STATE_FOCUSED);
  QT_CHECK(atspi_state_set_equals(s1, s2), "equal sets compare equal");
  atspi_state_set_add(s2, ATSPI_STATE_CHECKED);
  AtspiStateSet *diff = atspi_state_set_compare(s1, s2);
  GArray *ds = atspi_state_set_get_states(diff);
  QT_CHECK(ds && ds->len == 1 && g_array_index(ds, AtspiStateType, 0) == ATSPI_STATE_CHECKED,
           "compare() returns the one differing state");
  if (ds) g_array_free(ds, TRUE);
  g_object_unref(diff); g_object_unref(s1); g_object_unref(s2);
  QT_END();
}
