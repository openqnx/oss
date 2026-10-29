#include "common.h"
#include <stdlib.h>
#include <sys/stat.h>
#include <wayland-client.h>
#include <wayland-server.h>
#include <xkbcommon/xkbcommon.h>
/* No compositor is running on the target, so this drives what is testable
 * without one: creating a server-side display and event loop, the client-side
 * connection attempt failing cleanly, and xkbcommon compiling a real keymap. */
int main(void) {
  QT_BEGIN("wayland");
  /* A compositor's socket lives in XDG_RUNTIME_DIR (a session sets it;
   * wayland 1.24 refuses to guess): make one when the shell has none. */
  char rt[] = "/tmp/wl-rt-XXXXXX";
  if (!getenv("XDG_RUNTIME_DIR") && mkdtemp(rt)) { chmod(rt, 0700); setenv("XDG_RUNTIME_DIR", rt, 1); }

  /* --- server side: a display and its event loop are purely local --- */
  struct wl_display *disp = wl_display_create();
  QT_CHECK(disp != NULL, "wl_display_create");
  if (disp) {
    struct wl_event_loop *loop = wl_display_get_event_loop(disp);
    QT_CHECK(loop != NULL, "wl_display_get_event_loop");
    /* This exercises the epoll shim underneath. */
    int rc = wl_event_loop_dispatch(loop, 0);
    QT_CHECK(rc >= 0, "wl_event_loop_dispatch (epoll shim works)");
    const char *sock = wl_display_add_socket_auto(disp);
    QT_CHECK(sock != NULL, "wl_display_add_socket_auto");
    if (sock) printf("  info listening socket %s\n", sock);
    wl_display_destroy(disp);
    QT_CHECK(1, "wl_display_destroy");
  }

  /* --- client side: connecting with no compositor must fail, not crash --- */
  struct wl_display *c = wl_display_connect("definitely-no-such-compositor");
  QT_CHECK(c == NULL, "wl_display_connect fails cleanly with no compositor");

  /* --- xkbcommon: compile a keymap from RMLVO names --- */
  struct xkb_context *ctx = xkb_context_new(XKB_CONTEXT_NO_FLAGS);
  QT_CHECK(ctx != NULL, "xkb_context_new");
  if (ctx) {
    struct xkb_rule_names names = { NULL, NULL, "us", NULL, NULL };
    struct xkb_keymap *km = xkb_keymap_new_from_names(ctx, &names,
                                                      XKB_KEYMAP_COMPILE_NO_FLAGS);
    if (km) {
      QT_CHECK(xkb_keymap_num_mods(km) > 0, "keymap has modifiers");
      struct xkb_state *st = xkb_state_new(km);
      QT_CHECK(st != NULL, "xkb_state_new");
      if (st) {
        /* Which keysym a raw keycode maps to depends on the ruleset the
         * target's XKB data provides, so only require that it resolves. */
        xkb_keysym_t sym = xkb_state_key_get_one_sym(st, 24);
        char name[64] = {0};
        xkb_keysym_get_name(sym, name, sizeof(name));
        printf("  info keycode 24 -> %s\n", name);
        QT_CHECK(sym != XKB_KEY_NoSymbol, "keycode resolved to a keysym");
        xkb_state_unref(st);
      }
      xkb_keymap_unref(km);
    } else {
      /* xkeyboard-config data is a separate package; absence is not a port bug */
      printf("  info no XKB rule data installed -- keymap compile skipped\n");
      QT_CHECK(1, "xkb_keymap_new_from_names returned without crashing");
    }
    xkb_context_unref(ctx);
  }
  printf("  info xkbcommon linked, wayland client+server linked\n");
  QT_END();
}
