/* libxkbcommon + libxkbregistry: keysym names, case mapping and UTF-8;
 * compile an evdev/pc105 "us,de" keymap from xkeyboard-config's data, then
 * drive an xkb_state: plain and shifted letters, Caps Lock, switching to
 * the German group (Y key gives z), modifier queries; serialise the keymap
 * to text and recompile it from that string; list layouts with
 * libxkbregistry. */
#include "common.h"
#include <stdlib.h>
#include <xkbcommon/xkbcommon.h>
#include <xkbcommon/xkbregistry.h>
#define EVDEV(k) ((k) + 8)
enum { KEY_Y = 21, KEY_A = 30, KEY_LEFTSHIFT = 42, KEY_CAPSLOCK = 58, KEY_1 = 2 };
static xkb_keysym_t sym(struct xkb_state *st, int key) { return xkb_state_key_get_one_sym(st, EVDEV(key)); }
static void tap(struct xkb_state *st, int key) {
  xkb_state_update_key(st, EVDEV(key), XKB_KEY_DOWN); xkb_state_update_key(st, EVDEV(key), XKB_KEY_UP);
}
int main(void) {
  QT_BEGIN("libxkbcommon");
  QT_CHECK(xkb_keysym_from_name("Return", 0) == XKB_KEY_Return && xkb_keysym_from_name("eacute", 0) == XKB_KEY_eacute,
           "xkb_keysym_from_name");
  QT_CHECK(xkb_keysym_from_name("RETURN", XKB_KEYSYM_CASE_INSENSITIVE) == XKB_KEY_Return, "case-insensitive lookup");
  char nm[64] = "";
  xkb_keysym_get_name(XKB_KEY_KP_Enter, nm, sizeof nm);
  QT_CHECK(strcmp(nm, "KP_Enter") == 0, "xkb_keysym_get_name KP_Enter");
  char u8[8] = "";
  xkb_keysym_to_utf8(XKB_KEY_eacute, u8, sizeof u8);
  QT_CHECK(strcmp(u8, "\xc3\xa9") == 0 && xkb_keysym_to_utf32(XKB_KEY_EuroSign) == 0x20ac, "keysym to UTF-8 / UTF-32");
  QT_CHECK(xkb_keysym_to_upper(XKB_KEY_ssharp) != XKB_KEY_NoSymbol && xkb_keysym_to_upper(XKB_KEY_a) == XKB_KEY_A &&
           xkb_keysym_to_lower(XKB_KEY_Odiaeresis) == XKB_KEY_odiaeresis && xkb_utf32_to_keysym(0x3b1) == XKB_KEY_Greek_alpha,
           "case mapping and utf32_to_keysym");

  struct xkb_context *ctx = xkb_context_new(XKB_CONTEXT_NO_FLAGS);
  QT_CHECK(ctx != NULL, "xkb_context_new");
  if (!ctx) QT_END();
  for (unsigned i = 0; i < xkb_context_num_include_paths(ctx); i++)
    printf("  info include path %s\n", xkb_context_include_path_get(ctx, i));
  struct xkb_rule_names names = { "evdev", "pc105", "us,de", "", "" };
  struct xkb_keymap *km = xkb_keymap_new_from_names(ctx, &names, XKB_KEYMAP_COMPILE_NO_FLAGS);
  QT_CHECK(km != NULL, "keymap compiled from xkeyboard-config (evdev, pc105, us,de)");
  if (km) {
    QT_CHECK(xkb_keymap_num_layouts(km) == 2, "two layouts");
    const char *l0 = xkb_keymap_layout_get_name(km, 0), *l1 = xkb_keymap_layout_get_name(km, 1);
    printf("  info layouts: %s | %s\n", l0 ? l0 : "(null)", l1 ? l1 : "(null)");
    QT_CHECK(l0 && strstr(l0, "English") && l1 && strstr(l1, "German"), "layout names English / German");
    QT_CHECK(xkb_keymap_mod_get_index(km, XKB_MOD_NAME_SHIFT) != XKB_MOD_INVALID &&
             xkb_keymap_mod_get_index(km, XKB_MOD_NAME_CTRL) != XKB_MOD_INVALID, "Shift and Control modifiers exist");
    struct xkb_state *st = xkb_state_new(km);
    QT_CHECK(sym(st, KEY_A) == XKB_KEY_a && sym(st, KEY_Y) == XKB_KEY_y && sym(st, KEY_1) == XKB_KEY_1, "us: a, y, 1");
    xkb_state_update_key(st, EVDEV(KEY_LEFTSHIFT), XKB_KEY_DOWN);
    QT_CHECK(sym(st, KEY_A) == XKB_KEY_A && sym(st, KEY_1) == XKB_KEY_exclam, "Shift: A, !");
    QT_CHECK(xkb_state_mod_name_is_active(st, XKB_MOD_NAME_SHIFT, XKB_STATE_MODS_EFFECTIVE) == 1, "Shift reported active");
    char buf[8] = "";
    xkb_state_key_get_utf8(st, EVDEV(KEY_A), buf, sizeof buf);
    QT_CHECK(strcmp(buf, "A") == 0, "key_get_utf8 with Shift");
    xkb_state_update_key(st, EVDEV(KEY_LEFTSHIFT), XKB_KEY_UP);
    tap(st, KEY_CAPSLOCK);
    QT_CHECK(sym(st, KEY_A) == XKB_KEY_A && xkb_state_mod_name_is_active(st, XKB_MOD_NAME_CAPS, XKB_STATE_MODS_LOCKED) == 1,
             "Caps Lock latches upper case");
    tap(st, KEY_CAPSLOCK);
    QT_CHECK(sym(st, KEY_A) == XKB_KEY_a, "Caps Lock released");
    xkb_state_update_mask(st, 0, 0, 0, 0, 0, 1);   /* locked group 1 = de */
    QT_CHECK(xkb_state_serialize_layout(st, XKB_STATE_LAYOUT_EFFECTIVE) == 1 && sym(st, KEY_Y) == XKB_KEY_z,
             "German group: Y key gives z");
    xkb_state_unref(st);

    char *text = xkb_keymap_get_as_string(km, XKB_KEYMAP_FORMAT_TEXT_V1);
    QT_CHECK(text && strstr(text, "xkb_keymap") && strstr(text, "xkb_symbols"), "keymap serialised to text");
    struct xkb_keymap *k2 = text ? xkb_keymap_new_from_string(ctx, text, XKB_KEYMAP_FORMAT_TEXT_V1, 0) : NULL;
    QT_CHECK(k2 && xkb_keymap_num_layouts(k2) == 2, "keymap recompiled from its own text");
    if (k2) {
      struct xkb_state *s2 = xkb_state_new(k2);
      xkb_state_update_mask(s2, 0, 0, 0, 0, 0, 1);
      QT_CHECK(sym(s2, KEY_Y) == XKB_KEY_z, "recompiled keymap behaves the same");
      xkb_state_unref(s2); xkb_keymap_unref(k2);
    }
    if (text) printf("  info serialised keymap: %zu bytes\n", strlen(text));
    free(text);
    xkb_keymap_unref(km);
  }
  printf("  info expected: xkbcommon errors for a missing layout follow\n");
  names.layout = "no-such-layout-qnx";
  struct xkb_keymap *bad = xkb_keymap_new_from_names(ctx, &names, 0);
  QT_CHECK(bad == NULL, "unknown layout fails to compile");
  if (bad) xkb_keymap_unref(bad);
  xkb_context_unref(ctx);

  struct rxkb_context *rx = rxkb_context_new(RXKB_CONTEXT_NO_FLAGS);
  QT_CHECK(rx && rxkb_context_parse(rx, "evdev"), "xkbregistry parses the evdev ruleset");
  int found_us = 0, n = 0;
  if (rx) {
    for (struct rxkb_layout *l = rxkb_layout_first(rx); l; l = rxkb_layout_next(l)) {
      n++;
      const char *ln = rxkb_layout_get_name(l), *v = rxkb_layout_get_variant(l), *desc = rxkb_layout_get_description(l);
      if (ln && strcmp(ln, "us") == 0 && !v && desc) { found_us = 1; printf("  info us: %s\n", desc); }
    }
    printf("  info %d layouts in the registry\n", n);
    rxkb_context_unref(rx);
  }
  QT_CHECK(n > 50 && found_us, "registry lists layout 'us' among many");
  QT_END();
}
