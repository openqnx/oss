/* libxfce4util: write an XfceRc (keyfile) in /tmp with string, int, bool and
 * list entries in two groups, flush, reopen read-only and read everything
 * back; parse a hand-written file with translated keys; string helpers
 * (xfce_str_replace, UTF-8 control-char removal, desktop-entry unescape,
 * variable expansion) and xfce_mkdirhier, with known answers. */
#include "common.h"
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <libxfce4util/libxfce4util.h>
int main(void) {
  QT_BEGIN("libxfce4util");
  printf("  info libxfce4util %s (header %d.%d.%d)\n", xfce_version_string(), LIBXFCE4UTIL_MAJOR_VERSION,
         LIBXFCE4UTIL_MINOR_VERSION, LIBXFCE4UTIL_MICRO_VERSION);
  QT_CHECK(LIBXFCE4UTIL_CHECK_VERSION(4, 16, 0), "version >= 4.16");

  char dir[64]; snprintf(dir, sizeof dir, "/tmp/qnxport-xfce4util-%d", (int)getpid());
  gchar *sub = g_build_filename(dir, "a", "b", "c", NULL);
  GError *err = NULL;
  QT_CHECK(xfce_mkdirhier(sub, 0755, &err) && g_file_test(sub, G_FILE_TEST_IS_DIR), "xfce_mkdirhier a/b/c");
  if (err) { printf("  info %s\n", err->message); g_clear_error(&err); }
  gchar *file = g_build_filename(dir, "test.rc", NULL);

  XfceRc *rc = xfce_rc_simple_open(file, FALSE);
  QT_CHECK(rc != NULL, "xfce_rc_simple_open (read-write, new file)");
  if (rc) {
    xfce_rc_set_group(rc, "Panel");
    xfce_rc_write_entry(rc, "name", "QNX panel");
    xfce_rc_write_int_entry(rc, "size", 48);
    xfce_rc_write_bool_entry(rc, "autohide", TRUE);
    gchar *items[] = { "launcher", "clock", "tasklist", NULL };
    xfce_rc_write_list_entry(rc, "plugins", items, ";");
    xfce_rc_set_group(rc, "Second");
    xfce_rc_write_int_entry(rc, "negative", -7);
    QT_CHECK(xfce_rc_is_dirty(rc), "rc dirty after writes");
    xfce_rc_flush(rc);
    QT_CHECK(!xfce_rc_is_dirty(rc), "clean after flush");
    xfce_rc_close(rc);
  }
  gchar *text = NULL;
  QT_CHECK(g_file_get_contents(file, &text, NULL, NULL) && strstr(text, "[Panel]") && strstr(text, "size=48"),
           "file on disk has [Panel] and size=48");
  if (text) printf("  info file:\n%s", text);
  g_free(text);

  rc = xfce_rc_simple_open(file, TRUE);
  QT_CHECK(rc && xfce_rc_is_readonly(rc), "reopen read-only");
  if (rc) {
    QT_CHECK(xfce_rc_has_group(rc, "Panel") && xfce_rc_has_group(rc, "Second"), "both groups present");
    xfce_rc_set_group(rc, "Panel");
    const gchar *n = xfce_rc_read_entry(rc, "name", NULL);
    QT_CHECK(n && strcmp(n, "QNX panel") == 0, "string entry");
    QT_CHECK(xfce_rc_read_int_entry(rc, "size", 0) == 48, "int entry");
    QT_CHECK(xfce_rc_read_bool_entry(rc, "autohide", FALSE) == TRUE, "bool entry");
    gchar **l = xfce_rc_read_list_entry(rc, "plugins", ";");
    QT_CHECK(l && g_strv_length(l) == 3 && strcmp(l[1], "clock") == 0, "list entry (3 items)");
    g_strfreev(l);
    QT_CHECK(xfce_rc_read_int_entry(rc, "missing", 99) == 99, "default for a missing key");
    xfce_rc_set_group(rc, "Second");
    QT_CHECK(xfce_rc_read_int_entry(rc, "negative", 0) == -7, "negative int in second group");
    xfce_rc_close(rc);
  }

  gchar *f2 = g_build_filename(dir, "app.desktop", NULL);
  g_file_set_contents(f2, "[Desktop Entry]\nName=Terminal\nName[de]=Konsole\nExec=xfce4-terminal\n# comment\n", -1, NULL);
  rc = xfce_rc_simple_open(f2, TRUE);
  QT_CHECK(rc != NULL, "parse a desktop file");
  if (rc) {
    xfce_rc_set_group(rc, "Desktop Entry");
    const gchar *n = xfce_rc_read_entry_untranslated(rc, "Name", NULL);
    const gchar *e = xfce_rc_read_entry(rc, "Exec", NULL);
    QT_CHECK(n && strcmp(n, "Terminal") == 0 && e && strcmp(e, "xfce4-terminal") == 0, "untranslated Name + Exec");
    gchar **ent = xfce_rc_get_entries(rc, "Desktop Entry");
    int cnt = ent ? (int)g_strv_length(ent) : 0;
    printf("  info %d entries in [Desktop Entry]\n", cnt);
    QT_CHECK(cnt >= 2, "xfce_rc_get_entries");
    g_strfreev(ent);
    xfce_rc_close(rc);
  }

  gchar *s = xfce_str_replace("a-b--c-", "-", "+");
  QT_CHECK(s && strcmp(s, "a+b++c+") == 0, "xfce_str_replace");
  g_free(s);
  gchar ctl[] = "a\001b\033c";
  xfce_utf8_remove_controls(ctl, -1, NULL);
  printf("  info remove_controls -> \"%s\"\n", ctl);
  QT_CHECK(strcmp(ctl, "a b c") == 0, "xfce_utf8_remove_controls replaces controls with spaces");
  s = xfce_unescape_desktop_entry_value("one\\stwo\\tthree\\\\");
  QT_CHECK(s && strcmp(s, "one two\tthree\\") == 0, "xfce_unescape_desktop_entry_value");
  g_free(s);
  g_setenv("QNXPORT_XFCE_VAR", "/opt/qnx", TRUE);
  s = xfce_expand_variables("$QNXPORT_XFCE_VAR/bin", NULL);
  printf("  info expand -> %s\n", s ? s : "(null)");
  QT_CHECK(s && strcmp(s, "/opt/qnx/bin") == 0, "xfce_expand_variables $VAR");
  g_free(s);

  unlink(file); unlink(f2);
  for (int i = 0; i < 4; i++) { rmdir(sub); gchar *up = g_path_get_dirname(sub); g_free(sub); sub = up; }
  g_free(file); g_free(f2); g_free(sub);
  QT_END();
}
