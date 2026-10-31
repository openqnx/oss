/* libxfce4ui.
 *  - without a display: version API, desktop-file icon lookup through the
 *    XDG data dirs (the shipped xfce4-about.desktop, and one this test
 *    writes with an absolute icon path -> a GFileIcon), action-entry tables;
 *  - on a private headless Wayland display (labwc): widgets -- an
 *    XfceTitledDialog with mixed buttons and a subtitle, a mixed button, a
 *    frame box, a menu built from action entries whose item really
 *    activates its callback -- and xfce_spawn_command_line() running a
 *    child; plus xfce4-about --version, which calls gtk_init first.
 *    Without labwc, xfce4-about must stop cleanly at the display probe. */
#include "common.h"
/* ---- shared by the g4 GUI-component tests (pasted, not included: every test
 * is one self-contained file). run() executes a command line through /bin/sh
 * with a time limit and collects its output; spawn_bg()/stop_bg() manage a
 * daemon; wl_up() starts a private headless Wayland display -- labwc on
 * wlroots' headless backend, as tests/src/test_labwc.c does -- for programs
 * that open a display before they will even print their version.  Children
 * are started with posix_spawn, not fork: GLib may already have threads. */
#include <errno.h>
#include <fcntl.h>
#include <poll.h>
#include <signal.h>
#include <spawn.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>
extern char **environ;
#define QT_U __attribute__((unused))

/* Everything the test starts is registered here and stopped on every way
 * out: normal exit (atexit), a fatal signal, or the overall time limit
 * (alarm).  Children are started as process-group leaders, so a whole group
 * -- labwc together with the Xwayland it starts -- goes at once.  Only
 * groups this test created are signalled; nothing is found by name. */
#ifndef QT_GUARD_SECS
#define QT_GUARD_SECS 300
#endif
#define QT_MAXBG 32
static pid_t qt_bg[QT_MAXBG];
static volatile sig_atomic_t qt_cleaning = 0;
static int qt_guarded = 0;
static QT_U void qt_reg(pid_t p) {
  for (int i = 0; i < QT_MAXBG; i++) if (qt_bg[i] <= 0) { qt_bg[i] = p; return; }
}
static QT_U void qt_unreg(pid_t p) {
  for (int i = 0; i < QT_MAXBG; i++) if (qt_bg[i] == p) qt_bg[i] = 0;
}
static QT_U void qt_nap(long ms) { struct timespec t = { ms / 1000, (ms % 1000) * 1000000L }; nanosleep(&t, NULL); }
/* async-signal-safe: kill, waitpid, nanosleep only */
static QT_U void qt_kill_all(void) {
  if (qt_cleaning) return;
  qt_cleaning = 1;
  int any = 0, st;
  for (int i = 0; i < QT_MAXBG; i++) if (qt_bg[i] > 0) { kill(-qt_bg[i], SIGTERM); kill(qt_bg[i], SIGTERM); any = 1; }
  for (int t = 0; any && t < 30; t++) {          /* up to 3 s to go on SIGTERM */
    any = 0;
    for (int i = 0; i < QT_MAXBG; i++) {
      if (qt_bg[i] <= 0) continue;
      pid_t r = waitpid(qt_bg[i], &st, WNOHANG);
      if (r == qt_bg[i] || (r < 0 && errno == ECHILD)) { kill(-qt_bg[i], SIGKILL); qt_bg[i] = 0; }
      else any = 1;
    }
    if (any) qt_nap(100);
  }
  for (int i = 0; i < QT_MAXBG; i++) if (qt_bg[i] > 0) {
    kill(-qt_bg[i], SIGKILL); kill(qt_bg[i], SIGKILL); waitpid(qt_bg[i], &st, 0); qt_bg[i] = 0;
  }
  qt_cleaning = 0;
}
static QT_U void qt_on_signal(int sig) {
  static const char m1[] = "  FAIL overall time limit reached -- stopping everything this test started\n";
  static const char m2[] = "  info fatal signal -- stopping everything this test started\n";
  if (sig == SIGALRM) { ssize_t w = write(1, m1, sizeof m1 - 1); (void)w; }
  else { ssize_t w = write(1, m2, sizeof m2 - 1); (void)w; }
  qt_kill_all();
  if (sig == SIGALRM) {
    static const char m3[] = "FAIL (timeout)\n";
    ssize_t w = write(1, m3, sizeof m3 - 1); (void)w;
    _exit(124);
  }
  signal(sig, SIG_DFL);
  raise(sig);
  _exit(128 + sig);
}
static QT_U void qt_guard(void) {
  if (qt_guarded) return;
  qt_guarded = 1;
  atexit(qt_kill_all);
  static const int sigs[] = { SIGINT, SIGTERM, SIGHUP, SIGQUIT, SIGALRM, SIGSEGV, SIGBUS, SIGABRT, SIGFPE, SIGILL };
  for (size_t i = 0; i < sizeof sigs / sizeof sigs[0]; i++) {
    struct sigaction sa; memset(&sa, 0, sizeof sa);
    sa.sa_handler = qt_on_signal; sigemptyset(&sa.sa_mask);
    sigaction(sigs[i], &sa, NULL);
  }
  alarm(QT_GUARD_SECS);
}
static QT_U double qt_now(void) {
  struct timespec t; clock_gettime(CLOCK_MONOTONIC, &t);
  return (double)t.tv_sec + t.tv_nsec / 1e9;
}
/* Exit status, 128+N for death by signal N, -1 on timeout / spawn failure. */
static QT_U int run(const char *cmd, char *out, size_t n, int secs) {
  int p[2]; size_t len = 0; int status = 0, done = 0, eof = 0;
  out[0] = '\0';
  qt_guard();
  if (pipe(p) != 0) return -1;
  posix_spawn_file_actions_t fa; posix_spawnattr_t at;
  posix_spawn_file_actions_init(&fa);
  posix_spawn_file_actions_addclose(&fa, p[0]);
  posix_spawn_file_actions_adddup2(&fa, p[1], 1);
  posix_spawn_file_actions_adddup2(&fa, p[1], 2);
  posix_spawn_file_actions_addclose(&fa, p[1]);
  posix_spawn_file_actions_addopen(&fa, 0, "/dev/null", O_RDONLY, 0);
  posix_spawnattr_init(&at);
  posix_spawnattr_setflags(&at, POSIX_SPAWN_SETPGROUP);
  posix_spawnattr_setpgroup(&at, 0);
  char *argv[] = { "sh", "-c", (char *)cmd, NULL };
  pid_t pid;
  int rc = posix_spawn(&pid, "/bin/sh", &fa, &at, argv, environ);
  posix_spawn_file_actions_destroy(&fa); posix_spawnattr_destroy(&at);
  close(p[1]);
  if (rc != 0) { close(p[0]); return -1; }
  qt_reg(pid);
  double t0 = qt_now(), tdone = 0;
  int timed_out = 0;
  while (!(done && (eof || qt_now() - tdone > 1.0))) {
    struct pollfd pf = { p[0], POLLIN, 0 };
    if (!eof && poll(&pf, 1, 100) > 0) {
      char buf[4096]; ssize_t r = read(p[0], buf, sizeof buf);
      if (r > 0) {
        size_t c = (size_t)r; if (len + c > n - 1) c = n - 1 - len;
        memcpy(out + len, buf, c); len += c; out[len] = '\0';
      } else if (r == 0 || errno != EINTR) eof = 1;
    } else if (eof) usleep(50000);
    if (!done && waitpid(pid, &status, WNOHANG) == pid) { done = 1; tdone = qt_now(); }
    if (!done && qt_now() - t0 > secs) {
      kill(-pid, SIGKILL); kill(pid, SIGKILL); waitpid(pid, &status, 0);
      done = 1; timed_out = 1; tdone = qt_now();
    }
  }
  close(p[0]);
  kill(-pid, SIGKILL);   /* anything the command left running in its group */
  qt_unreg(pid);
  if (timed_out) { printf("  info timed out after %d s: %s\n", secs, cmd); return -1; }
  if (WIFEXITED(status)) return WEXITSTATUS(status);
  if (WIFSIGNALED(status)) return 128 + WTERMSIG(status);
  return -1;
}
/* A daemon, its output appended to log; returns its pid or -1. */
static QT_U pid_t spawn_bg(char *const argv[], const char *log) {
  qt_guard();
  posix_spawn_file_actions_t fa; posix_spawnattr_t at;
  posix_spawnattr_init(&at);
  posix_spawnattr_setflags(&at, POSIX_SPAWN_SETPGROUP);   /* its own group: stopped as one */
  posix_spawnattr_setpgroup(&at, 0);
  posix_spawn_file_actions_init(&fa);
  posix_spawn_file_actions_addopen(&fa, 0, "/dev/null", O_RDONLY, 0);
  posix_spawn_file_actions_addopen(&fa, 1, log, O_WRONLY | O_CREAT | O_APPEND, 0644);
  posix_spawn_file_actions_adddup2(&fa, 1, 2);
  pid_t pid;
  int rc = posix_spawnp(&pid, argv[0], &fa, &at, argv, environ);
  posix_spawn_file_actions_destroy(&fa); posix_spawnattr_destroy(&at);
  if (rc != 0) { printf("  info cannot start %s: %s\n", argv[0], strerror(rc)); return -1; }
  qt_reg(pid);
  return pid;
}
/* SIGTERM, up to 3 s to exit, then SIGKILL. Returns 1 if it exited on SIGTERM. */
static QT_U int stop_bg(pid_t *p) {
  int st, ok = 0;
  if (*p <= 0) return 0;
  kill(*p, SIGTERM);
  for (int i = 0; i < 30; i++) {
    if (waitpid(*p, &st, WNOHANG) == *p) { ok = 1; break; }
    usleep(100000);
  }
  if (!ok) { kill(-*p, SIGKILL); kill(*p, SIGKILL); waitpid(*p, &st, 0); }
  /* the rest of its group (labwc's Xwayland, ...): SIGTERM, a moment, SIGKILL */
  if (kill(-*p, SIGTERM) == 0) { usleep(300000); kill(-*p, SIGKILL); }
  qt_unreg(*p);
  *p = -1;
  return ok;
}
/* Wait for a path to appear; give up early if the child meant to create it died. */
static QT_U int wait_path(const char *path, int tenths, pid_t *child) {
  struct stat s; int st;
  while (tenths-- > 0) {
    if (stat(path, &s) == 0) return 1;
    if (child && *child > 0 && waitpid(*child, &st, WNOHANG) == *child) {
      printf("  info pid %d exited early (%s %d)\n", (int)*child,
             WIFEXITED(st) ? "status" : "signal", WIFEXITED(st) ? WEXITSTATUS(st) : WTERMSIG(st));
      kill(-*child, SIGKILL); qt_unreg(*child);
      *child = -1; return 0;
    }
    usleep(100000);
  }
  return 0;
}
/* The last few lines of a log, as info lines. */
static QT_U void show_log(const char *path, int lines) {
  static char buf[16384]; FILE *f = fopen(path, "r"); size_t n = 0;
  if (!f) return;
  fseek(f, 0, SEEK_END); long sz = ftell(f);
  if (sz > (long)sizeof buf - 1) fseek(f, sz - (long)sizeof buf + 1, SEEK_SET); else fseek(f, 0, SEEK_SET);
  n = fread(buf, 1, sizeof buf - 1, f); fclose(f); buf[n] = '\0';
  char *s = buf + n; int k = 0;
  while (s > buf && k <= lines) { --s; if (*s == '\n') ++k; }
  if (*s == '\n') ++s;
  for (char *l = strtok(s, "\n"); l; l = strtok(NULL, "\n")) printf("  info   | %s\n", l);
}
static QT_U int write_file(const char *path, const char *text) {
  FILE *f = fopen(path, "w"); if (!f) return 0;
  int ok = fputs(text, f) >= 0; return fclose(f) == 0 && ok;
}
static QT_U int file_has(const char *path, const char *needle) {
  static char buf[65536]; FILE *f = fopen(path, "r"); if (!f) return 0;
  size_t n = fread(buf, 1, sizeof buf - 1, f); fclose(f); buf[n] = '\0';
  return strstr(buf, needle) != NULL;
}
/* Output, first line only, for info lines. */
static QT_U const char *line1(const char *s) {
  static char b[256]; size_t i = 0;
  while (s[i] && s[i] != '\n' && i < sizeof b - 1) { b[i] = s[i]; i++; }
  b[i] = '\0'; return b;
}
/* A scratch directory, 0700, used as HOME, XDG_RUNTIME_DIR and the XDG
 * config/data/cache homes, so nothing reads or writes the real ones. */
static char qt_tmp[96];
static QT_U int scratch_up(const char *tag) {
  char p[200];
  qt_guard();
  snprintf(qt_tmp, sizeof qt_tmp, "/tmp/qnxport-%s.XXXXXX", tag);
  if (!mkdtemp(qt_tmp)) { printf("  info mkdtemp failed: %s\n", strerror(errno)); return 0; }
  chmod(qt_tmp, 0700);
  setenv("HOME", qt_tmp, 1); setenv("XDG_RUNTIME_DIR", qt_tmp, 1);
  snprintf(p, sizeof p, "%s/config", qt_tmp); mkdir(p, 0700); setenv("XDG_CONFIG_HOME", p, 1);
  snprintf(p, sizeof p, "%s/data", qt_tmp);   mkdir(p, 0700); setenv("XDG_DATA_HOME", p, 1);
  snprintf(p, sizeof p, "%s/cache", qt_tmp);  mkdir(p, 0700); setenv("XDG_CACHE_HOME", p, 1);
  setenv("XDG_DATA_DIRS", "/usr/local/share:/usr/share", 1);
  setenv("XDG_CONFIG_DIRS", "/usr/local/etc/xdg:/etc/xdg", 1);
  /* No session bus: fail at once rather than try to autolaunch one. */
  snprintf(p, sizeof p, "unix:path=%s/no-such-bus", qt_tmp);
  setenv("DBUS_SESSION_BUS_ADDRESS", p, 1);
  unsetenv("DISPLAY"); unsetenv("WAYLAND_DISPLAY");
  setenv("LC_ALL", "C", 1); setenv("LANG", "C", 1); unsetenv("LANGUAGE");
  return 1;
}
static QT_U void scratch_down(void) {
  char cmd[160]; static char out[256];
  if (!qt_tmp[0]) return;
  snprintf(cmd, sizeof cmd, "rm -rf '%s'", qt_tmp); run(cmd, out, sizeof out, 20);
}
static QT_U pid_t qt_labwc = -1;
static QT_U int wl_up(void) {
  char log[160], sock[160];
  snprintf(log, sizeof log, "%s/labwc.log", qt_tmp);
  snprintf(sock, sizeof sock, "%s/wayland-0", qt_tmp);
  setenv("WLR_BACKENDS", "headless", 1); setenv("WLR_RENDERER", "pixman", 1);
  char *lab[] = { "labwc", NULL };
  qt_labwc = spawn_bg(lab, log);
  if (qt_labwc <= 0 || !wait_path(sock, 150, &qt_labwc)) {
    printf("  info labwc did not come up (%s)\n", sock); show_log(log, 10);
    if (qt_labwc > 0) stop_bg(&qt_labwc);
    return 0;
  }
  setenv("WAYLAND_DISPLAY", "wayland-0", 1); setenv("GDK_BACKEND", "wayland", 1);
  return 1;
}
static QT_U void wl_down(void) { if (qt_labwc > 0) stop_bg(&qt_labwc); unsetenv("WAYLAND_DISPLAY"); }
/* ---- end of shared helpers ---- */
#include <libxfce4ui/libxfce4ui.h>

enum { ACT_OPEN = 7, ACT_SAVE = 9 };
static int fired = 0;
static void on_action(GObject *param) { (void)param; fired++; }

/* A desktop file whose Icon is an absolute path to a real PNG.  Written
 * before the first desktop-id lookup: GIO indexes the application
 * directories once, at that first lookup. */
static void prep_icon_files(void) {
  char path[300], cmd[400];
  snprintf(path, sizeof path, "%s/data/applications", qt_tmp); mkdir(path, 0700);
  snprintf(path, sizeof path, "%s/data/applications/qnx-port-test.desktop", qt_tmp);
  snprintf(cmd, sizeof cmd, "[Desktop Entry]\nType=Application\nName=QNX port test\nExec=true\nIcon=%s/qnx.png\n", qt_tmp);
  write_file(path, cmd);
  /* a real image behind the absolute Icon path */
  snprintf(path, sizeof path, "%s/qnx.png", qt_tmp);
  GdkPixbuf *pb = gdk_pixbuf_new(GDK_COLORSPACE_RGB, TRUE, 8, 16, 16);
  gdk_pixbuf_fill(pb, 0x3366ccff);
  GError *perr = NULL;
  if (!gdk_pixbuf_save(pb, path, "png", &perr, NULL)) {
    printf("  info cannot write %s: %s\n", path, perr ? perr->message : "?"); g_clear_error(&perr);
  }
  g_object_unref(pb);
}

/* xfce_gicon_from_name() confirms every icon through the default GtkIconTheme,
 * which needs a GdkScreen: only call it once GTK is up on a display. */
static void check_icons(void) {
  /* GTK finds the hicolor icons libxfce4ui installs only through hicolor's
     index.theme (package hicolor-icon-theme). */
  if (access("/usr/local/share/icons/hicolor/index.theme", R_OK) != 0)
    printf("  info /usr/local/share/icons/hicolor/index.theme missing (hicolor-icon-theme not installed)\n");
  GIcon *gi = xfce_gicon_from_name("xfce4-about");
  QT_CHECK(gi && G_IS_THEMED_ICON(gi), "xfce_gicon_from_name: themed icon for a named icon");
  if (gi) {
    const gchar *const *names = g_themed_icon_get_names(G_THEMED_ICON(gi));
    QT_CHECK(names && names[0] && strcmp(names[0], "org.xfce.about") == 0, "themed icon names start with org.xfce.about");
    g_object_unref(gi);
  }
  gi = xfce_gicon_from_name("qnx-port-test");
  QT_CHECK(gi && G_IS_FILE_ICON(gi), "desktop file with an absolute Icon path -> GFileIcon");
  if (gi) {
    char *p = g_file_get_path(g_file_icon_get_file(G_FILE_ICON(gi)));
    char want[200]; snprintf(want, sizeof want, "%s/qnx.png", qt_tmp);
    QT_CHECK(p && strcmp(p, want) == 0, "GFileIcon points at that path");
    g_free(p); g_object_unref(gi);
  }

}

int main(int argc, char **argv) {
  QT_BEGIN("libxfce4ui");
  static char out[16384];
  char path[300], cmd[400];
  if (!scratch_up("libxfce4ui")) { QT_CHECK(0, "scratch directory"); QT_END(); }
  prep_icon_files();

  printf("  info libxfce4ui %u.%u.%u\n", libxfce4ui_major_version, libxfce4ui_minor_version, libxfce4ui_micro_version);
  QT_CHECK(libxfce4ui_major_version == 4 && libxfce4ui_minor_version == 20 && libxfce4ui_micro_version == 0,
           "libxfce4ui 4.20.0");
  QT_CHECK(libxfce4ui_check_version(4, 20, 0) == NULL && libxfce4ui_check_version(4, 99, 0) != NULL,
           "libxfce4ui_check_version");

  /* --- icons from desktop files --- */
  gchar *icon = xfce_icon_name_from_desktop_id("xfce4-about");
  printf("  info xfce4-about.desktop Icon=%s\n", icon ? icon : "(none)");
  QT_CHECK(icon && strcmp(icon, "org.xfce.about") == 0, "xfce_icon_name_from_desktop_id(xfce4-about) = org.xfce.about");
  g_free(icon);
  /* --- action entries --- */
  XfceGtkActionEntry entries[] = {
    { ACT_OPEN, "<Actions>/QnxTest/open", "<Primary>o", XFCE_GTK_MENU_ITEM, "_Open", "Open a file", "document-open", G_CALLBACK(on_action) },
    { ACT_SAVE, "<Actions>/QnxTest/save", "<Primary>s", XFCE_GTK_MENU_ITEM, "_Save", "Save the file", "document-save", G_CALLBACK(on_action) },
  };
  xfce_gtk_translate_action_entries(entries, G_N_ELEMENTS(entries));
  const XfceGtkActionEntry *e = xfce_gtk_get_action_entry_by_id(entries, G_N_ELEMENTS(entries), ACT_SAVE);
  QT_CHECK(e && strcmp(e->menu_item_label_text, "_Save") == 0 && strcmp(e->default_accelerator, "<Primary>s") == 0,
           "xfce_gtk_get_action_entry_by_id finds the entry (labels kept in the C locale)");

  /* --- widgets and spawning, on a headless display --- */
  int disp = wl_up();
  printf("  info headless Wayland display: %s\n", disp ? "up" : "not available");
  int rc = run("xfce4-about --version", out, sizeof out, 60);
  printf("  info xfce4-about --version -> %d: %s\n", rc, line1(out));
  if (disp) {
    QT_CHECK(rc == 0 && strstr(out, "xfce4-about 4.20.0 (Xfce 4.20"), "xfce4-about --version: 4.20.0");
    int gtk_ok = gtk_init_check(&argc, &argv);
    QT_CHECK(gtk_ok, "gtk_init_check on the headless display");
    if (gtk_ok) {
      check_icons();
      GtkWidget *dlg = xfce_titled_dialog_new_with_mixed_buttons("QNX test", NULL, 0,
                         "document-save", "_Save", GTK_RESPONSE_OK, "", "_Cancel", GTK_RESPONSE_CANCEL, NULL);
      QT_CHECK(dlg && XFCE_IS_TITLED_DIALOG(dlg), "XfceTitledDialog with mixed buttons");
      if (dlg) {
        xfce_titled_dialog_set_subtitle(XFCE_TITLED_DIALOG(dlg), "a subtitle");
        const gchar *st = xfce_titled_dialog_get_subtitle(XFCE_TITLED_DIALOG(dlg));
        QT_CHECK(st && strcmp(st, "a subtitle") == 0, "subtitle set and read back");
        QT_CHECK(g_strcmp0(gtk_window_get_title(GTK_WINDOW(dlg)), "QNX test") == 0, "dialog title");
        GtkWidget *ok = gtk_dialog_get_widget_for_response(GTK_DIALOG(dlg), GTK_RESPONSE_OK);
        QT_CHECK(ok && g_strcmp0(gtk_button_get_label(GTK_BUTTON(ok)), "_Save") == 0, "OK response button is \"_Save\"");
        gtk_widget_destroy(dlg);
      }
      GtkWidget *mb = xfce_gtk_button_new_mixed("document-open", "_Open");
      g_object_ref_sink(mb);
      QT_CHECK(GTK_IS_BUTTON(mb) && g_strcmp0(gtk_button_get_label(GTK_BUTTON(mb)), "_Open") == 0, "xfce_gtk_button_new_mixed");
      g_object_unref(mb);
      GtkWidget *inner = NULL;
      GtkWidget *frame = xfce_gtk_frame_box_new("Frame title", &inner);
      g_object_ref_sink(frame);
      QT_CHECK(GTK_IS_FRAME(frame) && inner != NULL, "xfce_gtk_frame_box_new returns a frame and its container");
      g_object_unref(frame);
      GtkWidget *menu = gtk_menu_new();
      g_object_ref_sink(menu);
      GObject *param = G_OBJECT(g_simple_action_new("p", NULL));
      GtkWidget *mi = xfce_gtk_menu_item_new_from_action_entry(e, param, GTK_MENU_SHELL(menu));
      GList *kids = gtk_container_get_children(GTK_CONTAINER(menu));
      QT_CHECK(mi && g_list_length(kids) == 1, "menu item built from the action entry and appended");
      g_list_free(kids);
      if (mi) gtk_menu_item_activate(GTK_MENU_ITEM(mi));
      QT_CHECK(fired == 1, "activating the menu item runs the entry's callback");
      g_object_unref(menu); g_object_unref(param);

      GError *err = NULL;
      snprintf(path, sizeof path, "%s/spawned.txt", qt_tmp);
      snprintf(cmd, sizeof cmd, "sh -c 'echo spawned-by-xfce > %s'", path);
      gboolean sp = xfce_spawn_command_line(NULL, cmd, FALSE, FALSE, TRUE, &err);
      if (err) { printf("  info spawn error: %s\n", err->message); g_clear_error(&err); }
      int have = 0;
      for (int i = 0; i < 100 && sp && !(have = file_has(path, "spawned-by-xfce")); i++) {
        g_main_context_iteration(NULL, FALSE); usleep(50000);
      }
      QT_CHECK(sp && have, "xfce_spawn_command_line ran the child");
      /* let go of the display before labwc is stopped under it */
      gdk_display_close(gdk_display_get_default());
    }
  } else {
    printf("  info no display: xfce_gicon_from_name checks skipped (they need a GdkScreen)\n");
    QT_CHECK(rc != 0 && rc != -1 && (strstr(out, "isplay") || strstr(out, "GTK")),
             "without a display xfce4-about stops at the display probe");
  }
  wl_down();
  scratch_down();
  QT_END();
}
