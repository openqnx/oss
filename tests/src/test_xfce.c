/* The Xfce library stack: libxfce4util's string/rc helpers, xfconf's client
 * (which needs a session bus, so its absence is the expected result on a bare
 * target), garcon's .desktop menu parser and libxfce4ui's type registration. */
#include "common.h"
#include <libxfce4util/libxfce4util.h>
#include <xfconf/xfconf.h>
#include <garcon/garcon.h>
int main(void) {
  QT_BEGIN("xfce libraries");

  /* --- libxfce4util --- */
  gchar *s = xfce_str_replace("build zlib and zstd", "z", "Z");
  QT_CHECK(s && strcmp(s, "build Zlib and Zstd") == 0, "xfce_str_replace");
  g_free(s);
  gchar *joined = xfce_str_replace("a,b,c", ",", " / ");
  QT_CHECK(joined && strcmp(joined, "a / b / c") == 0, "multi-character replacement");
  g_free(joined);
  /* xfce_expand_variables only substitutes what its own environment handling
     recognises, so assert on what it is documented to guarantee: a string with
     nothing to expand comes back unchanged, never NULL. */
  gchar *exp = xfce_expand_variables("/opt/ports/qnx", NULL);
  QT_CHECK(exp && strcmp(exp, "/opt/ports/qnx") == 0,
           "xfce_expand_variables passes a plain path through");
  g_free(exp);

  XfceRc *rc = xfce_rc_simple_open("/tmp/qnxport-xfce-test.rc", FALSE);
  QT_CHECK(rc != NULL, "xfce_rc_simple_open for writing");
  if (rc) {
    xfce_rc_write_entry(rc, "Arch", "aarch64le");
    xfce_rc_write_int_entry(rc, "Sdp", 8);
    xfce_rc_flush(rc);
    xfce_rc_close(rc);
    rc = xfce_rc_simple_open("/tmp/qnxport-xfce-test.rc", TRUE);
    QT_CHECK(rc && g_strcmp0(xfce_rc_read_entry(rc, "Arch", NULL), "aarch64le") == 0,
             "rc file round trip (string)");
    QT_CHECK(rc && xfce_rc_read_int_entry(rc, "Sdp", 0) == 8, "rc file round trip (int)");
    if (rc) xfce_rc_close(rc);
    g_unlink("/tmp/qnxport-xfce-test.rc");
  }

  /* --- garcon: .desktop parsing needs no display --- */
  garcon_set_environment("XFCE");
  GarconMenuItem *item = garcon_menu_item_new_for_path("/nonexistent.desktop");
  QT_CHECK(item == NULL, "a missing .desktop file is rejected cleanly");

  /* --- xfconf: requires a session bus --- */
  GError *err = NULL;
  if (xfconf_init(&err)) {
    printf("  info a session bus is available\n");
    XfconfChannel *ch = xfconf_channel_get("qnx-port-test");
    QT_CHECK(XFCONF_IS_CHANNEL(ch), "got an xfconf channel");
    xfconf_shutdown();
  } else {
    printf("  info no session bus: %s\n", err ? err->message : "(no message)");
    QT_CHECK(err != NULL, "xfconf reports the missing bus rather than aborting");
    g_clear_error(&err);
  }
  QT_END();
}
