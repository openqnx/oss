/* xfce4-panel.
 *  - libxfce4panel: version API, the enum types plugins are configured with,
 *    and the XfcePanelPlugin class's properties;
 *  - the eleven plugins the panel ships, loaded the way the panel loads them
 *    (panel/panel-module.c): each share/xfce4/panel/plugins/<p>.desktop names
 *    X-XFCE-Module, lib<module>.so is opened with GModule, and its
 *    xfce_panel_module_init() registers a plugin GType -- an XfcePanelPlugin
 *    subclass -- into a GTypeModule;
 *  - xfce4-panel calls gtk_init() before it parses --version, so the test
 *    starts a private headless Wayland display (labwc) for it, and on that
 *    display also builds an XfceArrowButton.  Without labwc, the panel must
 *    stop cleanly at the display probe. */
#include "common.h"
#include <gmodule.h>
#include <libxfce4panel/libxfce4panel.h>
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

/* A GTypeModule for an already-opened GModule, as the panel's PanelModule is. */
typedef struct { GTypeModule parent; } QtMod;
typedef struct { GTypeModuleClass parent; } QtModClass;
G_DEFINE_TYPE(QtMod, qt_mod, G_TYPE_TYPE_MODULE)
static gboolean qt_mod_load(GTypeModule *m) { (void)m; return TRUE; }
static void qt_mod_unload(GTypeModule *m) { (void)m; }
static void qt_mod_class_init(QtModClass *k) {
  G_TYPE_MODULE_CLASS(k)->load = qt_mod_load; G_TYPE_MODULE_CLASS(k)->unload = qt_mod_unload;
}
static void qt_mod_init(QtMod *m) { (void)m; }

typedef GType (*InitFunc)(GTypeModule *, gboolean *);

static const char *enum_nick(GType t, int v) {
  GEnumClass *k = g_type_class_ref(t);
  GEnumValue *ev = g_enum_get_value(k, v);
  const char *n = ev ? ev->value_nick : "?";
  g_type_class_unref(k);
  return n;
}

int main(int argc, char **argv) {
  QT_BEGIN("xfce4-panel");
  static char out[16384];

  printf("  info libxfce4panel %u.%u.%u\n", libxfce4panel_major_version, libxfce4panel_minor_version,
         libxfce4panel_micro_version);
  QT_CHECK(libxfce4panel_major_version == 4 && libxfce4panel_minor_version == 20 && libxfce4panel_micro_version == 0,
           "libxfce4panel 4.20.0");
  QT_CHECK(libxfce4panel_check_version(4, 20, 0) == NULL && libxfce4panel_check_version(4, 99, 0) != NULL,
           "libxfce4panel_check_version");
  unsetenv("XFCE_PANEL_CHANNEL_NAME");
  QT_CHECK(strcmp(xfce_panel_get_channel_name(), "xfce4-panel") == 0, "xfconf channel name xfce4-panel");
  QT_CHECK(strcmp(enum_nick(XFCE_TYPE_PANEL_PLUGIN_MODE, XFCE_PANEL_PLUGIN_MODE_DESKBAR), "deskbar") == 0,
           "XfcePanelPluginMode enum: deskbar");
  QT_CHECK(strcmp(enum_nick(XFCE_TYPE_SCREEN_POSITION, XFCE_SCREEN_POSITION_FLOATING_V), "floating-v") == 0,
           "XfceScreenPosition enum: floating-v");
  GObjectClass *pk = g_type_class_ref(XFCE_TYPE_PANEL_PLUGIN);
  const char *props[] = { "name", "display-name", "comment", "unique-id", "arguments", "orientation",
                          "size", "icon-size", "small", "screen-position", "expand", "shrink", "mode", "nrows" };
  int np = 0;
  for (size_t i = 0; i < G_N_ELEMENTS(props); i++) if (g_object_class_find_property(pk, props[i])) np++;
  QT_CHECK(np == (int)G_N_ELEMENTS(props), "XfcePanelPlugin has its 14 documented properties");
  g_type_class_unref(pk);

  /* --- the plugins --- */
  const char *mods[] = { "actions", "applicationsmenu", "clock", "directorymenu", "launcher", "pager",
                         "separator", "showdesktop", "systray", "tasklist", "windowmenu" };
  int loaded = 0;
  for (size_t i = 0; i < G_N_ELEMENTS(mods); i++) {
    char *desk = g_strdup_printf("/usr/local/share/xfce4/panel/plugins/%s.desktop", mods[i]);
    GKeyFile *kf = g_key_file_new();
    char *module = NULL, *api = NULL;
    if (g_key_file_load_from_file(kf, desk, 0, NULL)) {
      module = g_key_file_get_string(kf, "Xfce Panel", "X-XFCE-Module", NULL);
      api = g_key_file_get_string(kf, "Xfce Panel", "X-XFCE-API", NULL);
    }
    char *so = g_strdup_printf("/usr/local/lib/xfce4/panel/plugins/lib%s.so", module ? module : mods[i]);
    GModule *gm = g_module_open(so, G_MODULE_BIND_LOCAL);
    InitFunc init = NULL;
    GType t = 0;
    if (gm && g_module_symbol(gm, "xfce_panel_module_init", (gpointer *)&init) && init) {
      GTypeModule *tm = g_object_new(qt_mod_get_type(), NULL);
      g_type_module_set_name(tm, mods[i]);
      g_type_module_use(tm);
      gboolean resident = FALSE;
      t = init(tm, &resident);
      g_module_make_resident(gm);   /* its types are registered now; never unload */
    }
    int ok = module && strcmp(module, mods[i]) == 0 && api && strcmp(api, "2.0") == 0 &&
             t != 0 && g_type_is_a(t, XFCE_TYPE_PANEL_PLUGIN);
    if (ok) loaded++;
    printf("  info %-16s module %s, API %s -> %s%s\n", mods[i], module ? module : "-", api ? api : "-",
           t ? g_type_name(t) : "no type", gm ? "" : g_module_error());
    g_free(desk); g_free(module); g_free(api); g_free(so); g_key_file_free(kf);
  }
  QT_CHECK(loaded == (int)G_N_ELEMENTS(mods),
           "all 11 plugins: .desktop -> module -> xfce_panel_module_init registers an XfcePanelPlugin type");
  QT_CHECK(file_has("/usr/local/etc/xdg/xfce4/panel/default.xml", "<channel name=\"xfce4-panel\""),
           "default panel layout (xfconf XML) installed");

  /* --- the program, on a private headless display --- */
  if (!scratch_up("xfce4-panel")) { QT_CHECK(0, "scratch directory"); QT_END(); }
  int disp = wl_up();
  printf("  info headless Wayland display: %s\n", disp ? "up" : "not available");
  int rc = run("xfce4-panel --version", out, sizeof out, 60);
  printf("  info xfce4-panel --version -> %d: %s\n", rc, line1(out));
  if (disp) {
    QT_CHECK(rc == 0 && strstr(out, "xfce4-panel 4.20.0 (Xfce 4.20"), "xfce4-panel --version: 4.20.0");
    rc = run("xfce4-panel --help", out, sizeof out, 60);
    QT_CHECK(rc == 0 && strstr(out, "--preferences") && strstr(out, "--add"), "xfce4-panel --help lists --preferences, --add");
    if (gtk_init_check(&argc, &argv)) {
      GtkWidget *ab = xfce_arrow_button_new(GTK_ARROW_UP);
      g_object_ref_sink(ab);
      QT_CHECK(xfce_arrow_button_get_arrow_type(XFCE_ARROW_BUTTON(ab)) == GTK_ARROW_UP, "XfceArrowButton: arrow up");
      xfce_arrow_button_set_arrow_type(XFCE_ARROW_BUTTON(ab), GTK_ARROW_LEFT);
      xfce_arrow_button_set_blinking(XFCE_ARROW_BUTTON(ab), TRUE);
      QT_CHECK(xfce_arrow_button_get_arrow_type(XFCE_ARROW_BUTTON(ab)) == GTK_ARROW_LEFT &&
               xfce_arrow_button_get_blinking(XFCE_ARROW_BUTTON(ab)), "XfceArrowButton: set arrow left, blinking");
      xfce_arrow_button_set_blinking(XFCE_ARROW_BUTTON(ab), FALSE);
      g_object_unref(ab);
      /* let go of the display before labwc is stopped under it */
      gdk_display_close(gdk_display_get_default());
    } else QT_CHECK(0, "gtk_init_check on the headless display");
  } else {
    QT_CHECK(rc != 0 && rc != -1 && (strstr(out, "isplay") || strstr(out, "GTK")),
             "without a display xfce4-panel stops at the display probe");
  }
  wl_down();
  scratch_down();
  QT_END();
}
