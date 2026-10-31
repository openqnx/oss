/* linux-headers-input-codes (<linux/input-event-codes.h>): the evdev event
 * types and codes libinput, libxkbcommon and wlroots are written against.
 * Check known values, then use them the way a compositor does: an evdev key
 * code + 8 is an XKB keycode, and libxkbcommon (with the xkeyboard-config
 * data, when installed) must map KEY_A, KEY_ENTER, KEY_1 to their keysyms. */
#include "common.h"
#include <linux/input-event-codes.h>
#include <xkbcommon/xkbcommon.h>
#include <stdlib.h>
#include <unistd.h>

int main(void) {
  QT_BEGIN("linux-headers-input-codes");
  QT_CHECK(EV_SYN == 0 && EV_KEY == 1 && EV_REL == 2 && EV_ABS == 3 && EV_MAX == 0x1f, "event types SYN KEY REL ABS, EV_MAX");
  QT_CHECK(KEY_ESC == 1 && KEY_A == 30 && KEY_Z == 44 && KEY_ENTER == 28 && KEY_SPACE == 57, "KEY_ESC 1, KEY_A 30, KEY_Z 44, ENTER 28, SPACE 57");
  QT_CHECK(KEY_LEFTSHIFT == 42 && KEY_F1 == 59 && KEY_F12 == 88 && KEY_MAX == 0x2ff && KEY_CNT == KEY_MAX + 1, "shift, F1, F12, KEY_MAX 0x2ff");
  QT_CHECK(BTN_LEFT == 0x110 && BTN_RIGHT == 0x111 && BTN_TOUCH == 0x14a && BTN_SOUTH == 0x130 && BTN_GAMEPAD == BTN_SOUTH,
           "BTN_LEFT 0x110, BTN_TOUCH 0x14a, BTN_SOUTH = BTN_GAMEPAD");
  QT_CHECK(REL_X == 0 && REL_WHEEL == 8 && ABS_MT_SLOT == 0x2f && ABS_MT_POSITION_X == 0x35 && ABS_MT_TRACKING_ID == 0x39,
           "REL_WHEEL 8, multitouch ABS_MT_* codes");
  QT_CHECK(SYN_REPORT == 0 && SYN_DROPPED == 3 && INPUT_PROP_DIRECT == 1 && SW_LID == 0 && LED_CAPSL == 1, "SYN, INPUT_PROP, SW, LED codes");

  const char *roots[] = { getenv("XKB_CONFIG_ROOT"), "/usr/local/share/X11/xkb", "/data/oss/usr/local/share/X11/xkb" };
  const char *root = NULL;
  for (int i = 0; i < 3 && !root; i++) if (roots[i]) { char p[512]; snprintf(p, sizeof p, "%s/symbols/us", roots[i]); if (access(p, R_OK) == 0) root = roots[i]; }
  if (!root) { printf("  info no XKB data installed: evdev -> keysym mapping not exercised\n"); QT_END(); }
  struct xkb_context *ctx = xkb_context_new(XKB_CONTEXT_NO_DEFAULT_INCLUDES | XKB_CONTEXT_NO_ENVIRONMENT_NAMES);
  xkb_context_include_path_append(ctx, root);
  struct xkb_rule_names names = { "evdev", "pc105", "us", "", "" };
  struct xkb_keymap *km = xkb_keymap_new_from_names(ctx, &names, XKB_KEYMAP_COMPILE_NO_FLAGS);
  QT_CHECK(km != NULL, "US keymap compiled");
  if (km) {
    struct xkb_state *st = xkb_state_new(km);
    QT_CHECK(xkb_state_key_get_one_sym(st, KEY_A + 8) == XKB_KEY_a, "KEY_A + 8 -> keysym a");
    QT_CHECK(xkb_state_key_get_one_sym(st, KEY_ENTER + 8) == XKB_KEY_Return, "KEY_ENTER + 8 -> Return");
    QT_CHECK(xkb_state_key_get_one_sym(st, KEY_1 + 8) == XKB_KEY_1, "KEY_1 + 8 -> 1");
    xkb_state_update_key(st, KEY_LEFTSHIFT + 8, XKB_KEY_DOWN);
    QT_CHECK(xkb_state_key_get_one_sym(st, KEY_A + 8) == XKB_KEY_A, "with KEY_LEFTSHIFT held: KEY_A -> A");
    xkb_state_unref(st);
    xkb_keymap_unref(km);
  }
  xkb_context_unref(ctx);
  QT_END();
}
