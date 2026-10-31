/* xkeyboard-config (the XKB keymap database): libxkbcommon compiles keymaps
 * from it through its rules, as every Wayland compositor does -- US, German
 * (QWERTZ, dead keys), French (AZERTY), Russian (Cyrillic) and a
 * multi-layout keymap with a group switch option -- and the keysyms of known
 * keys must come out. The rules registry (evdev.xml) must list them. */
#include "common.h"
#include <stdlib.h>
#include <unistd.h>
#include <xkbcommon/xkbcommon.h>

#define KC(evdev) ((evdev) + 8)
enum { K_Q = 16, K_Y = 21, K_A = 30, K_Z = 44, K_SEMICOLON = 39, K_1 = 2, K_EQUAL = 13, K_LSHIFT = 42, K_RALT = 100 };

static struct xkb_context *ctx;
static struct xkb_keymap *keymap(const char *layout, const char *variant, const char *options) {
  struct xkb_rule_names n = { "evdev", "pc105", layout, variant, options };
  return xkb_keymap_new_from_names(ctx, &n, XKB_KEYMAP_COMPILE_NO_FLAGS);
}
static xkb_keysym_t sym(struct xkb_keymap *km, int evdev, int shift) {
  struct xkb_state *st = xkb_state_new(km);
  if (shift) xkb_state_update_key(st, KC(K_LSHIFT), XKB_KEY_DOWN);
  xkb_keysym_t s = xkb_state_key_get_one_sym(st, KC(evdev));
  xkb_state_unref(st);
  return s;
}

int main(void) {
  QT_BEGIN("xkeyboard-config");
  const char *roots[] = { getenv("XKB_CONFIG_ROOT"), "/usr/local/share/X11/xkb", "/data/oss/usr/local/share/X11/xkb" };
  const char *root = NULL;
  for (int i = 0; i < 3 && !root; i++) if (roots[i]) { char p[512]; snprintf(p, sizeof p, "%s/rules/evdev", roots[i]); if (access(p, R_OK) == 0) root = roots[i]; }
  QT_CHECK(root != NULL, "XKB data (rules/evdev)");
  if (!root) QT_END();
  printf("  info %s\n", root);
  ctx = xkb_context_new(XKB_CONTEXT_NO_DEFAULT_INCLUDES | XKB_CONTEXT_NO_ENVIRONMENT_NAMES);
  QT_CHECK(ctx && xkb_context_include_path_append(ctx, root), "xkb context with only this data");

  struct xkb_keymap *us = keymap("us", "", ""), *de = keymap("de", "", ""), *fr = keymap("fr", "", ""), *ru = keymap("ru", "", "");
  QT_CHECK(us && de && fr && ru, "keymaps us, de, fr, ru compiled");
  if (us) {
    QT_CHECK(sym(us, K_A, 0) == XKB_KEY_a && sym(us, K_A, 1) == XKB_KEY_A && sym(us, K_SEMICOLON, 0) == XKB_KEY_semicolon, "us: a, A, semicolon");
    QT_CHECK(sym(us, K_1, 1) == XKB_KEY_exclam, "us: shift+1 -> exclam");
    QT_CHECK(xkb_keymap_num_layouts(us) == 1 && xkb_keymap_mod_get_index(us, XKB_MOD_NAME_SHIFT) != XKB_MOD_INVALID, "us: one layout, Shift modifier");
  }
  if (de) {
    QT_CHECK(sym(de, K_Y, 0) == XKB_KEY_z && sym(de, K_Z, 0) == XKB_KEY_y, "de: QWERTZ (Y key -> z, Z key -> y)");
    QT_CHECK(sym(de, K_SEMICOLON, 0) == XKB_KEY_odiaeresis && sym(de, K_EQUAL, 0) == XKB_KEY_dead_acute, "de: odiaeresis, dead_acute");
  }
  if (fr) QT_CHECK(sym(fr, K_Q, 0) == XKB_KEY_a && sym(fr, K_A, 0) == XKB_KEY_q && sym(fr, K_1, 0) == XKB_KEY_ampersand, "fr: AZERTY (Q key -> a, 1 -> ampersand)");
  if (ru) QT_CHECK(sym(ru, K_A, 0) == XKB_KEY_Cyrillic_ef && sym(ru, K_Q, 1) == XKB_KEY_Cyrillic_SHORTI, "ru: Cyrillic ef, SHORTI");

  struct xkb_keymap *multi = keymap("us,de", "", "grp:alt_shift_toggle");
  QT_CHECK(multi && xkb_keymap_num_layouts(multi) == 2, "us,de keymap: 2 layouts");
  if (multi) {
    const char *n1 = xkb_keymap_layout_get_name(multi, 1);
    printf("  info layout 1: %s\n", n1 ? n1 : "-");
    struct xkb_state *st = xkb_state_new(multi);
    xkb_state_update_mask(st, 0, 0, 0, 0, 0, 1);         /* lock group 2 */
    QT_CHECK(xkb_state_key_get_one_sym(st, KC(K_Y)) == XKB_KEY_z, "group 2 (de): Y key -> z");
    xkb_state_unref(st);
    char *txt = xkb_keymap_get_as_string(multi, XKB_KEYMAP_FORMAT_TEXT_V1);
    QT_CHECK(txt && strstr(txt, "xkb_symbols") && strstr(txt, "<AC01>"), "keymap serialised to text");
    struct xkb_keymap *back = txt ? xkb_keymap_new_from_string(ctx, txt, XKB_KEYMAP_FORMAT_TEXT_V1, 0) : NULL;
    QT_CHECK(back && xkb_keymap_num_layouts(back) == 2, "and compiled back");
    if (back) xkb_keymap_unref(back);
    free(txt);
    xkb_keymap_unref(multi);
  }
  /* the registry the configuration tools read */
  char p[512]; snprintf(p, sizeof p, "%s/rules/evdev.xml", root);
  FILE *f = fopen(p, "r"); long found = 0; char line[512];
  while (f && fgets(line, sizeof line, f)) if (strstr(line, "<name>us</name>") || strstr(line, "<name>de</name>") || strstr(line, "<name>fr</name>")) found++;
  if (f) fclose(f);
  QT_CHECK(found >= 3, "rules/evdev.xml lists us, de, fr");
  if (us) xkb_keymap_unref(us);
  if (de) xkb_keymap_unref(de);
  if (fr) xkb_keymap_unref(fr);
  if (ru) xkb_keymap_unref(ru);
  xkb_context_unref(ctx);
  QT_END();
}
