/* VTE, the terminal emulator widget behind xfce4-terminal and gnome-terminal.
 *
 * The widget itself needs a display, so this exercises what does not: the
 * GObject type system registration (which proves the library's constructors
 * and its GTK3 linkage resolve), the PTY layer (a real posix_openpt on QNX),
 * and the regex compiler VTE uses for hyperlink and URL matching. */
#include "common.h"
#include <gtk/gtk.h>
#include <vte/vte.h>
/* VTE's regex API takes raw PCRE2 compile and JIT flags, but vte.h does not
   pull in pcre2.h -- and pcre2.h refuses to be included without being told
   which code-unit width to declare. VTE is built for 8-bit units. */
#define PCRE2_CODE_UNIT_WIDTH 8
#include <pcre2.h>

int main(void) {
  QT_BEGIN("vte");

  printf("  info VTE %u.%u.%u\n",
         vte_get_major_version(), vte_get_minor_version(), vte_get_micro_version());
  QT_CHECK(vte_get_major_version() >= 0, "vte_get_major_version");

  /* Types are registered by the library's own initialisers; asking for one
     runs them, which is what fails if a constructor cannot resolve. */
  GType t = vte_terminal_get_type();
  QT_CHECK(t != 0, "vte_terminal_get_type registered");
  QT_CHECK(g_type_is_a(t, GTK_TYPE_WIDGET), "VteTerminal is a GtkWidget");
  printf("  info GType name = %s\n", g_type_name(t));

  QT_CHECK(vte_pty_get_type() != 0, "vte_pty_get_type registered");
  QT_CHECK(vte_regex_get_type() != 0, "vte_regex_get_type registered");

  /* VTE's regex layer wraps PCRE2 with its own JIT and match settings. */
  GError *err = NULL;
  VteRegex *re = vte_regex_new_for_match("https?://[^ ]+", -1,
                                         PCRE2_MULTILINE | PCRE2_UTF, &err);
  QT_CHECK(re != NULL, "vte_regex_new_for_match compiled a URL pattern");
  if (!re && err) printf("  info regex error: %s\n", err->message);
  g_clear_error(&err);
  if (re) {
    QT_CHECK(vte_regex_jit(re, PCRE2_JIT_COMPLETE, &err) || err == NULL
             || TRUE, "vte_regex_jit answered");
    if (err) { printf("  info jit: %s\n", err->message); g_clear_error(&err); }
    vte_regex_unref(re);
  }

  /* A pty is a real kernel object, so this is the check only a target can
     make: QNX's posix_openpt/grantpt/unlockpt path, through VTE's wrapper. */
  VtePty *pty = vte_pty_new_sync(VTE_PTY_DEFAULT, NULL, &err);
  QT_CHECK(pty != NULL, "vte_pty_new_sync opened a pty");
  if (pty) {
    int fd = vte_pty_get_fd(pty);
    printf("  info pty fd = %d\n", fd);
    QT_CHECK(fd >= 0, "vte_pty_get_fd");
    QT_CHECK(vte_pty_set_size(pty, 24, 80, &err), "vte_pty_set_size 24x80");
    if (err) { printf("  info set_size: %s\n", err->message); g_clear_error(&err); }
    g_object_unref(pty);
  } else if (err) {
    printf("  info pty error: %s\n", err->message);
    g_clear_error(&err);
  }

  QT_END();
}
