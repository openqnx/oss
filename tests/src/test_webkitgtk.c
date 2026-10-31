/* webkitgtk: the parts that work without a display.
 *  - JavaScriptCore through WebKitGTK's GLib API (libjavascriptcoregtk-4.1,
 *    not the JSCOnly port test_javascriptcore covers): scripts with known
 *    results, a C function called from JS, objects, exceptions, GC pressure.
 *  - the jsc shell WebKitGTK installs in libexec runs a script.
 *  - WebKitWebDriver serves the W3C WebDriver protocol on loopback: GET
 *    /status must answer {"value":{"ready":true,"message":"No sessions"}}.
 *  - libwebkit2gtk reports the 2.44.4 it was built as. */
#include "common.h"
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <webkit2/webkit2.h>
#include <jsc/jsc.h>
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

static int add_calls = 0;
static int js_add(int a, int b, gpointer data) { (void)data; add_calls++; return a + b; }

static int js_int(JSCContext *ctx, const char *code) {
  JSCValue *v = jsc_context_evaluate(ctx, code, -1);
  int r = (v && jsc_value_is_number(v)) ? jsc_value_to_int32(v) : -999999;
  if (v) g_object_unref(v);
  return r;
}
/* Result as a string, into a static buffer ("" on failure). */
static const char *js_str(JSCContext *ctx, const char *code) {
  static char b[512]; b[0] = '\0';
  JSCValue *v = jsc_context_evaluate(ctx, code, -1);
  if (v) { char *s = jsc_value_to_string(v); if (s) { g_strlcpy(b, s, sizeof b); g_free(s); } g_object_unref(v); }
  return b;
}

/* One HTTP request to 127.0.0.1:port; the whole response into out. */
static int http_get(int port, const char *path, char *out, size_t n) {
  out[0] = '\0';
  int fd = socket(AF_INET, SOCK_STREAM, 0); if (fd < 0) return 0;
  struct sockaddr_in a; memset(&a, 0, sizeof a);
  a.sin_family = AF_INET; a.sin_port = htons((unsigned short)port); a.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
  if (connect(fd, (struct sockaddr *)&a, sizeof a) != 0) { close(fd); return 0; }
  char req[256];
  int l = snprintf(req, sizeof req, "GET %s HTTP/1.1\r\nHost: 127.0.0.1:%d\r\nConnection: close\r\n\r\n", path, port);
  if (write(fd, req, (size_t)l) != l) { close(fd); return 0; }
  size_t len = 0; double t0 = qt_now();
  while (len < n - 1 && qt_now() - t0 < 10) {
    struct pollfd pf = { fd, POLLIN, 0 };
    if (poll(&pf, 1, 200) <= 0) { if (strstr(out, "}")) break; continue; }
    ssize_t r = read(fd, out + len, n - 1 - len);
    if (r <= 0) break;
    len += (size_t)r; out[len] = '\0';
  }
  close(fd);
  return len > 0;
}
static int port_open(int port) {
  int fd = socket(AF_INET, SOCK_STREAM, 0); if (fd < 0) return 0;
  struct sockaddr_in a; memset(&a, 0, sizeof a);
  a.sin_family = AF_INET; a.sin_port = htons((unsigned short)port); a.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
  int ok = connect(fd, (struct sockaddr *)&a, sizeof a) == 0;
  close(fd); return ok;
}

int main(void) {
  QT_BEGIN("webkitgtk");
  static char out[16384];

  printf("  info WebKitGTK %u.%u.%u, JavaScriptCoreGTK %u.%u.%u\n",
         webkit_get_major_version(), webkit_get_minor_version(), webkit_get_micro_version(),
         jsc_get_major_version(), jsc_get_minor_version(), jsc_get_micro_version());
  QT_CHECK(webkit_get_major_version() == 2 && webkit_get_minor_version() == 44 && webkit_get_micro_version() == 4,
           "libwebkit2gtk-4.1 is 2.44.4");
  QT_CHECK(jsc_get_major_version() == 2 && jsc_get_minor_version() == 44, "libjavascriptcoregtk-4.1 is 2.44");

  /* --- JavaScriptCore, GLib API --- */
  JSCContext *ctx = jsc_context_new();
  QT_CHECK(ctx != NULL, "jsc_context_new");
  if (ctx) {
    QT_CHECK(js_int(ctx, "6 * 7") == 42, "6 * 7 = 42");
    QT_CHECK(js_int(ctx, "(function f(n){return n<2?n:f(n-1)+f(n-2)})(20)") == 6765, "recursion: fib(20) = 6765");
    QT_CHECK(strcmp(js_str(ctx, "'QNX'.toLowerCase() + [3,1,2].sort().join('')"), "qnx123") == 0,
             "strings and Array.sort");
    QT_CHECK(strcmp(js_str(ctx, "JSON.stringify({a:[1,2], b:'x'})"), "{\"a\":[1,2],\"b\":\"x\"}") == 0,
             "JSON.stringify");
    QT_CHECK(strcmp(js_str(ctx, "/(\\d+)-(\\d+)/.exec('10-20')[2]"), "20") == 0, "RegExp capture group");
    QT_CHECK(strcmp(js_str(ctx, "new Date(Date.UTC(2024, 1, 29)).toISOString()"), "2024-02-29T00:00:00.000Z") == 0,
             "Date.UTC / toISOString (leap day)");
    QT_CHECK(js_int(ctx, "Math.round(Math.sqrt(1024) + Math.PI * 0)") == 32, "Math.sqrt");

    /* a C function the script calls */
    JSCValue *fn = jsc_value_new_function(ctx, "add", G_CALLBACK(js_add), NULL, NULL,
                                          G_TYPE_INT, 2, G_TYPE_INT, G_TYPE_INT);
    jsc_context_set_value(ctx, "add", fn);
    g_object_unref(fn);
    QT_CHECK(js_int(ctx, "add(40, 2) + add(-1, 1)") == 42 && add_calls == 2, "C callback called from JS (twice)");

    /* objects, both directions */
    JSCValue *obj = jsc_context_evaluate(ctx, "({ n: 5, name: 'qnx', list: [1, 2, 3] })", -1);
    QT_CHECK(obj && jsc_value_is_object(obj), "object literal");
    if (obj) {
      JSCValue *nv = jsc_value_object_get_property(obj, "n");
      QT_CHECK(nv && jsc_value_to_int32(nv) == 5, "object property read from C");
      if (nv) g_object_unref(nv);
      JSCValue *seven = jsc_value_new_number(ctx, 7);
      jsc_value_object_set_property(obj, "m", seven);
      g_object_unref(seven);
      jsc_context_set_value(ctx, "o", obj);
      QT_CHECK(js_int(ctx, "o.m * o.n + o.list.length") == 38, "property set from C, seen by JS");
      g_object_unref(obj);
    }

    /* exceptions reach the embedder */
    jsc_context_clear_exception(ctx);
    JSCValue *bad = jsc_context_evaluate(ctx, "null.x", -1);
    JSCException *ex = jsc_context_get_exception(ctx);
    QT_CHECK(ex != NULL, "TypeError reported as a JSCException");
    if (ex) {
      const char *m = jsc_exception_get_message(ex);
      const char *nm = jsc_exception_get_name(ex);
      printf("  info exception: %s: %s\n", nm ? nm : "(null)", m ? m : "(null)");
      QT_CHECK(nm && strcmp(nm, "TypeError") == 0, "exception name TypeError");
      jsc_context_clear_exception(ctx);
    }
    if (bad) g_object_unref(bad);

    QT_CHECK(js_int(ctx,
      "(function(){ let n = 0;"
      "  for (let i = 0; i < 100000; i++) { const o = {i, s: 'x'.repeat(8)}; n += o.i % 3; }"
      "  return n; })()") == 99999, "100k allocations, collected while running");
    g_object_unref(ctx);
  }

  if (!scratch_up("webkitgtk")) { QT_CHECK(0, "scratch directory"); QT_END(); }

  /* --- the jsc shell --- */
  char cmd[512], path[256];
  snprintf(path, sizeof path, "%s/t.js", qt_tmp);
  write_file(path, "var s = 0; for (var i = 1; i <= 100; i++) s += i;\n"
                   "print('sum=' + s + ' squares=' + [1,2,3].map(function (x) { return x * x; }).join(','));\n");
  snprintf(cmd, sizeof cmd, "/usr/local/libexec/webkit2gtk-4.1/jsc %s", path);
  int rc = run(cmd, out, sizeof out, 60);
  printf("  info jsc t.js -> %d: %s\n", rc, line1(out));
  QT_CHECK(rc == 0 && strstr(out, "sum=5050 squares=1,4,9"), "libexec jsc runs a script file");
  rc = run("/usr/local/libexec/webkit2gtk-4.1/jsc -e 'print(typeof WebAssembly, 2 ** 10)'", out, sizeof out, 60);
  printf("  info jsc -e -> %d: %s\n", rc, line1(out));
  QT_CHECK(rc == 0 && strstr(out, "1024"), "jsc -e evaluates an expression");

  /* --- WebKitWebDriver --- */
  rc = run("WebKitWebDriver --help", out, sizeof out, 30);
  QT_CHECK(strstr(out, "Usage: WebKitWebDriver") && strstr(out, "--port"), "WebKitWebDriver --help");
  int port = 47000 + (int)(getpid() % 2000);
  char parg[32]; snprintf(parg, sizeof parg, "--port=%d", port);
  char log[200]; snprintf(log, sizeof log, "%s/webdriver.log", qt_tmp);
  char *wd[] = { "WebKitWebDriver", parg, NULL };
  pid_t wp = spawn_bg(wd, log);
  int up = 0;
  for (int i = 0; i < 100 && wp > 0 && !up; i++) { up = port_open(port); if (!up) usleep(100000); }
  QT_CHECK(up, "WebKitWebDriver listens on 127.0.0.1");
  if (up) {
    int got = http_get(port, "/status", out, sizeof out);
    char *body = strstr(out, "\r\n\r\n");
    printf("  info GET /status: %s | %s\n", line1(out), body ? line1(body + 4) : "");
    QT_CHECK(got && strstr(out, " 200 "), "GET /status -> HTTP 200");
    QT_CHECK(strstr(out, "\"ready\":true") != NULL, "status: ready true");
    QT_CHECK(strstr(out, "No sessions") != NULL, "status: \"No sessions\"");
    got = http_get(port, "/session/no-such-session/url", out, sizeof out);
    printf("  info GET /session/<bogus>/url: %s\n", line1(out));
    QT_CHECK(got && strstr(out, " 404 ") && strstr(out, "invalid session id"), "unknown session -> 404 invalid session id");
  } else show_log(log, 10);
  stop_bg(&wp);

  scratch_down();
  QT_END();
}
