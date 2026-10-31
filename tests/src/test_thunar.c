/* thunar: the Xfce file manager, without a display.
 *  - libthunarx (the extension API): version check, menu items and menus
 *    as plain GObjects (properties, activation, submenus), and the provider
 *    factory loading thunar's own four extension modules from
 *    lib/thunarx-3 -- each provider type must be found and instantiated:
 *    renamers (thunar-sbr), property pages (thunar-apr), menu items
 *    (thunar-uca with its custom actions, thunar-wallpaper).
 *  - thunar --version / --help, answered before the application starts
 *    (GApplication handle-local-options), so no display is needed. */
#include "common.h"
#include <thunarx/thunarx.h>
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

static int activated = 0;
static void on_activate(ThunarxMenuItem *item, gpointer data) { (void)item; activated += GPOINTER_TO_INT(data); }

/* Instantiates the providers of one type and reports their type names. */
static int providers(GType iface, const char *want, char *names, size_t n) {
  ThunarxProviderFactory *f = thunarx_provider_factory_get_default();
  GList *list = thunarx_provider_factory_list_providers(f, iface);
  int found = 0; names[0] = '\0';
  for (GList *l = list; l; l = l->next) {
    const char *tn = G_OBJECT_TYPE_NAME(l->data);
    if (names[0]) g_strlcat(names, " ", n);
    g_strlcat(names, tn ? tn : "?", n);
    if (tn && strcmp(tn, want) == 0) found = 1;
  }
  g_list_free_full(list, g_object_unref);
  g_object_unref(f);
  return found;
}

int main(void) {
  QT_BEGIN("thunar");
  static char out[16384];
  printf("  info libthunarx %u.%u.%u\n", thunarx_major_version, thunarx_minor_version, thunarx_micro_version);
  QT_CHECK(thunarx_major_version == 4 && thunarx_minor_version == 20 && thunarx_micro_version == 0, "libthunarx 4.20.0");
  QT_CHECK(thunarx_check_version(4, 20, 0) == NULL, "thunarx_check_version(4.20.0) accepts");
  /* Upstream thunarx_check_version() is a stub that always returns NULL
   * (thunarx/thunarx-config.c), so there is no "refuses" case to test. */
  QT_CHECK(thunarx_check_version(5, 0, 0) == NULL, "thunarx_check_version(5.0.0): upstream stub, NULL");

  /* --- menus --- */
  ThunarxMenuItem *item = thunarx_menu_item_new("Qnx::Hello", "Say _hello", "Greets", "dialog-information");
  QT_CHECK(item != NULL, "thunarx_menu_item_new");
  gchar *nm = NULL, *label = NULL, *tip = NULL, *icon = NULL;
  g_object_get(item, "name", &nm, "label", &label, "tooltip", &tip, "icon", &icon, NULL);
  QT_CHECK(nm && strcmp(nm, "Qnx::Hello") == 0 && label && strcmp(label, "Say _hello") == 0 &&
           tip && strcmp(tip, "Greets") == 0 && icon && strcmp(icon, "dialog-information") == 0,
           "menu item properties read back");
  g_free(nm); g_free(label); g_free(tip); g_free(icon);
  QT_CHECK(thunarx_menu_item_get_sensitive(item), "sensitive by default");
  thunarx_menu_item_set_sensitive(item, FALSE);
  QT_CHECK(!thunarx_menu_item_get_sensitive(item), "set_sensitive(FALSE)");
  g_signal_connect(item, "activate", G_CALLBACK(on_activate), GINT_TO_POINTER(1));
  thunarx_menu_item_activate(item);
  thunarx_menu_item_activate(item);
  QT_CHECK(activated == 2, "\"activate\" emitted on each thunarx_menu_item_activate");

  ThunarxMenu *menu = thunarx_menu_new();
  ThunarxMenuItem *a = thunarx_menu_item_new("Qnx::A", "A", NULL, NULL);
  ThunarxMenuItem *b = thunarx_menu_item_new("Qnx::B", "B", NULL, NULL);
  thunarx_menu_append_item(menu, a);
  thunarx_menu_append_item(menu, b);
  thunarx_menu_prepend_item(menu, item);
  GList *items = thunarx_menu_get_items(menu);
  int order = g_list_length(items) == 3 && items->data == item && items->next->data == a &&
              items->next->next->data == b;
  QT_CHECK(order, "menu holds 3 items in order (prepend + 2 appends)");
  thunarx_menu_item_list_free(items);
  ThunarxMenuItem *top = thunarx_menu_item_new("Qnx::Top", "Top", NULL, NULL);
  thunarx_menu_item_set_menu(top, menu);
  ThunarxMenu *sub = NULL;
  g_object_get(top, "menu", &sub, NULL);
  QT_CHECK(sub == menu, "submenu attached to an item");
  if (sub) g_object_unref(sub);
  g_object_unref(top); g_object_unref(menu); g_object_unref(a); g_object_unref(b); g_object_unref(item);

  /* --- the extension modules --- */
  if (!scratch_up("thunar")) { QT_CHECK(0, "scratch directory"); QT_END(); }
  setenv("THUNARX_DIRS", "/usr/local/lib/thunarx-3", 1);
  char names[512];
  int ok = providers(THUNARX_TYPE_RENAMER_PROVIDER, "ThunarSbrProvider", names, sizeof names);
  printf("  info renamer providers: %s\n", names);
  QT_CHECK(ok, "thunar-sbr: ThunarSbrProvider (renamer provider) loaded");
  ok = providers(THUNARX_TYPE_PROPERTY_PAGE_PROVIDER, "ThunarAprProvider", names, sizeof names);
  printf("  info property page providers: %s\n", names);
  QT_CHECK(ok, "thunar-apr: ThunarAprProvider (property page provider) loaded");
  ok = providers(THUNARX_TYPE_MENU_PROVIDER, "ThunarUcaProvider", names, sizeof names);
  printf("  info menu providers: %s\n", names);
  QT_CHECK(ok, "thunar-uca: ThunarUcaProvider (custom actions) loaded");
  QT_CHECK(strstr(names, "TwpProvider") != NULL, "thunar-wallpaper: TwpProvider loaded");

  /* --- the program --- */
  int rc = run("thunar --version", out, sizeof out, 30);
  printf("  info thunar --version -> %d: %s\n", rc, line1(out));
  QT_CHECK(rc == 0 && strstr(out, "thunar 4.20.0 (Xfce 4.20"), "thunar --version: thunar 4.20.0 (Xfce 4.20)");
  rc = run("Thunar -V", out, sizeof out, 30);
  QT_CHECK(rc == 0 && strstr(out, "thunar 4.20.0"), "Thunar -V (the compatibility name)");
  rc = run("thunar --help", out, sizeof out, 30);
  QT_CHECK(rc == 0 && strstr(out, "--bulk-rename") && strstr(out, "--daemon"), "thunar --help lists --bulk-rename, --daemon");
  QT_CHECK(file_has("/usr/local/etc/xdg/Thunar/uca.xml", "<actions>"), "default custom actions (uca.xml) installed");
  scratch_down();
  QT_END();
}
