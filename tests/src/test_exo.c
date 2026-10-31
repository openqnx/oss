/* exo: Xfce's extension library, the parts that need no display -- version
 * API, string helpers, property bindings between plain GObjects, and the
 * GdkPixbuf effects (scaling with aspect ratio, translucency) with exact
 * pixel results.  exo-open opens the display while it parses options, so
 * its --version runs on a private headless Wayland display (labwc); without
 * one it must stop cleanly at the display probe. */
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
#include <exo/exo.h>
/* exo keeps its pre-4.18 helpers (deprecated for GLib equivalents); they are
 * still exported API, and are what is tested here. */
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"

static GdkPixbuf *solid(int w, int h, gboolean alpha, guint32 rgba) {
  GdkPixbuf *p = gdk_pixbuf_new(GDK_COLORSPACE_RGB, alpha, 8, w, h);
  gdk_pixbuf_fill(p, rgba);
  return p;
}
static void px(GdkPixbuf *p, int x, int y, guchar *c) {
  const guchar *d = gdk_pixbuf_get_pixels(p) + y * gdk_pixbuf_get_rowstride(p) + x * gdk_pixbuf_get_n_channels(p);
  for (int i = 0; i < gdk_pixbuf_get_n_channels(p); i++) c[i] = d[i];
}

int main(void) {
  QT_BEGIN("exo");
  static char out[16384];
  printf("  info exo %u.%u.%u\n", exo_major_version, exo_minor_version, exo_micro_version);
  QT_CHECK(exo_major_version == 4 && exo_minor_version == 20 && exo_micro_version == 0, "exo 4.20.0");
  /* Upstream exo 4.20 made exo_check_version() a stub that always returns
   * NULL (exo/exo-config.c); it is kept only for ABI compatibility. */
  QT_CHECK(exo_check_version(4, 20, 0) == NULL && exo_check_version(4, 99, 0) == NULL,
           "exo_check_version (upstream stub: always NULL)");

  /* --- strings --- */
  gchar *s = exo_str_elide_underscores("_File__Name_x");
  QT_CHECK(s && strcmp(s, "File_Namex") == 0, "exo_str_elide_underscores (\"__\" keeps one)");
  g_free(s);
  s = exo_str_replace("a.b.c", ".", "::");
  QT_CHECK(s && strcmp(s, "a::b::c") == 0, "exo_str_replace");
  g_free(s);
  QT_CHECK(exo_str_is_equal("x", "x") && !exo_str_is_equal("x", NULL) && exo_str_is_equal(NULL, NULL),
           "exo_str_is_equal (NULL-safe)");
  QT_CHECK(exo_str_looks_like_an_uri("https://example.org/") && exo_str_looks_like_an_uri("file:/tmp") &&
           !exo_str_looks_like_an_uri("/tmp/file") && !exo_str_looks_like_an_uri("1http://x"),
           "exo_str_looks_like_an_uri");
  QT_CHECK(exo_str_is_flag("--help") && exo_str_is_flag("-v") && !exo_str_is_flag("file"), "exo_str_is_flag");
  struct tm tm; memset(&tm, 0, sizeof tm);
  tm.tm_year = 2024 - 1900; tm.tm_mon = 1; tm.tm_mday = 29; tm.tm_hour = 13; tm.tm_min = 5; tm.tm_wday = 4;
  s = exo_strdup_strftime("%Y-%m-%d %H:%M", &tm);
  QT_CHECK(s && strcmp(s, "2024-02-29 13:05") == 0, "exo_strdup_strftime");
  g_free(s);
  gchar *v[] = { "one", "two", "three", NULL };
  gchar **d = exo_strndupv(v, 2);
  QT_CHECK(d && g_strv_length(d) == 2 && strcmp(d[1], "two") == 0, "exo_strndupv (first 2 of 3)");
  g_strfreev(d);

  /* --- bindings --- */
  GSimpleAction *a = g_simple_action_new("a", NULL), *b = g_simple_action_new("b", NULL),
                *c = g_simple_action_new("c", NULL);
  exo_binding_new(G_OBJECT(a), "enabled", G_OBJECT(b), "enabled");
  exo_binding_new_with_negation(G_OBJECT(a), "enabled", G_OBJECT(c), "enabled");
  g_simple_action_set_enabled(a, FALSE);
  QT_CHECK(!g_action_get_enabled(G_ACTION(b)) && g_action_get_enabled(G_ACTION(c)),
           "exo_binding: follows; with negation: inverts");
  g_simple_action_set_enabled(a, TRUE);
  QT_CHECK(g_action_get_enabled(G_ACTION(b)) && !g_action_get_enabled(G_ACTION(c)), "bindings track a second change");
  GSimpleAction *m1 = g_simple_action_new("m1", NULL), *m2 = g_simple_action_new("m2", NULL);
  exo_mutual_binding_new(G_OBJECT(m1), "enabled", G_OBJECT(m2), "enabled");
  g_simple_action_set_enabled(m2, FALSE);
  QT_CHECK(!g_action_get_enabled(G_ACTION(m1)), "exo_mutual_binding: target -> source");
  g_simple_action_set_enabled(m1, TRUE);
  QT_CHECK(g_action_get_enabled(G_ACTION(m2)), "exo_mutual_binding: source -> target");
  g_object_unref(a); g_object_unref(b); g_object_unref(c); g_object_unref(m1); g_object_unref(m2);

  /* --- pixbuf effects --- */
  GdkPixbuf *src = solid(100, 50, TRUE, 0x3366ccffu);
  GdkPixbuf *r = exo_gdk_pixbuf_scale_ratio(src, 20);
  QT_CHECK(r && gdk_pixbuf_get_width(r) == 20 && gdk_pixbuf_get_height(r) == 10, "scale_ratio 100x50 -> 20x10");
  if (r) g_object_unref(r);
  r = exo_gdk_pixbuf_scale_down(src, TRUE, 40, 40);
  QT_CHECK(r && gdk_pixbuf_get_width(r) == 40 && gdk_pixbuf_get_height(r) == 20, "scale_down 100x50 into 40x40 -> 40x20");
  if (r) g_object_unref(r);
  r = exo_gdk_pixbuf_scale_down(src, TRUE, 200, 200);
  QT_CHECK(r == src, "scale_down: already small enough -> the same pixbuf");
  if (r) g_object_unref(r);
  r = exo_gdk_pixbuf_lucent(src, 50);
  guchar cc[4] = { 0 };
  if (r) px(r, 7, 7, cc);
  printf("  info lucent 50%%: %02x%02x%02x%02x\n", cc[0], cc[1], cc[2], cc[3]);
  QT_CHECK(r && cc[0] == 0x33 && cc[1] == 0x66 && cc[2] == 0xcc && cc[3] == 127, "lucent 50%: colour kept, alpha 255 -> 127");
  if (r) g_object_unref(r);
  GdkPixbuf *opaque = solid(4, 4, FALSE, 0x102030ffu);
  r = exo_gdk_pixbuf_lucent(opaque, 20);
  if (r) px(r, 1, 1, cc);
  QT_CHECK(r && gdk_pixbuf_get_has_alpha(r) && cc[0] == 0x10 && cc[3] == 51, "lucent 20% on RGB: alpha channel added, 51");
  if (r) g_object_unref(r);
  g_object_unref(opaque); g_object_unref(src);

  /* --- exo-open --- */
  if (!scratch_up("exo")) { QT_CHECK(0, "scratch directory"); QT_END(); }
  int disp = wl_up();
  printf("  info headless Wayland display: %s\n", disp ? "up" : "not available");
  int rc = run("exo-open --version", out, sizeof out, 60);
  printf("  info exo-open --version -> %d: %s\n", rc, line1(out));
  if (disp) {
    QT_CHECK(rc == 0 && strstr(out, "exo-open 4.20.0"), "exo-open --version: exo-open 4.20.0");
    rc = run("exo-open --help", out, sizeof out, 60);
    QT_CHECK(rc == 0 && strstr(out, "--launch") && strstr(out, "WebBrowser"), "exo-open --help lists --launch TYPE");
  } else {
    QT_CHECK(rc != 0 && rc != -1 && (strstr(out, "isplay") || strstr(out, "GTK")),
             "without a display exo-open stops at the display probe");
  }
  wl_down();
  scratch_down();
  QT_END();
}
