/* xfconf: the Xfce configuration store, end to end on loopback.  The test
 * starts a private D-Bus session bus (dbus-daemon with its own config, a
 * UNIX socket in a scratch directory) and xfconfd on it, with
 * XDG_CONFIG_HOME pointing at a directory pre-seeded with one channel in
 * xfconfd's per-channel XML format.  Then:
 *  - xfconf-query reads the seeded values, creates, lists and resets
 *    properties;
 *  - libxfconf (in this process) reads what xfconf-query wrote, writes
 *    values xfconf-query reads back, and receives the "property-changed"
 *    signal for a change xfconf-query makes;
 *  - on SIGTERM xfconfd saves the new channel as XML.
 * Every wait is bounded; daemons are stopped by the PIDs this test started. */
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
#include <xfconf/xfconf.h>

#define XFCONFD "/usr/local/lib/xfce4/xfconf/xfconfd"
static int changed = 0;
static char changed_val[128];
static void on_changed(XfconfChannel *ch, const gchar *prop, const GValue *v, gpointer d) {
  (void)ch; (void)d;
  if (strcmp(prop, "/live/value") == 0) {
    changed++;
    if (v && G_VALUE_HOLDS_STRING(v) && g_value_get_string(v)) g_strlcpy(changed_val, g_value_get_string(v), sizeof changed_val);
  }
}
static const char *q(const char *args) {          /* xfconf-query <args>, trimmed output */
  static char out[8192]; char cmd[600];
  snprintf(cmd, sizeof cmd, "xfconf-query %s", args);
  int rc = run(cmd, out, sizeof out, 30);
  size_t n = strlen(out); while (n && (out[n - 1] == '\n' || out[n - 1] == ' ')) out[--n] = '\0';
  if (rc != 0) printf("  info xfconf-query %s -> %d: %s\n", args, rc, line1(out));
  return rc == 0 ? out : "";
}

int main(void) {
  QT_BEGIN("xfconf");
  static char out[8192];
  char path[300], text[1200], log[300];
  pid_t bus = -1, xd = -1;
  if (!scratch_up("xfconf")) { QT_CHECK(0, "scratch directory"); QT_END(); }

  int rc = run("xfconf-query --version", out, sizeof out, 30);
  printf("  info xfconf-query --version -> %d: %s\n", rc, line1(out));
  QT_CHECK(rc == 0 && strstr(out, "xfconf-query 4.20.0"), "xfconf-query --version: 4.20.0");
  rc = run(XFCONFD " --version", out, sizeof out, 30);
  printf("  info xfconfd --version -> %d: %s\n", rc, line1(out));
  QT_CHECK(rc == 0 && strstr(out, "4.20.0"), "xfconfd --version: 4.20.0");

  /* --- a seeded channel --- */
  const char *dirs[] = { "config/xfce4", "config/xfce4/xfconf", "config/xfce4/xfconf/xfce-perchannel-xml" };
  for (size_t i = 0; i < G_N_ELEMENTS(dirs); i++) { snprintf(path, sizeof path, "%s/%s", qt_tmp, dirs[i]); mkdir(path, 0700); }
  snprintf(path, sizeof path, "%s/config/xfce4/xfconf/xfce-perchannel-xml/qnx-seed.xml", qt_tmp);
  write_file(path,
    "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
    "<channel name=\"qnx-seed\" version=\"1.0\">\n"
    "  <property name=\"greeting\" type=\"string\" value=\"hello from xml\"/>\n"
    "  <property name=\"answer\" type=\"int\" value=\"42\"/>\n"
    "  <property name=\"nested\" type=\"empty\">\n"
    "    <property name=\"flag\" type=\"bool\" value=\"true\"/>\n"
    "  </property>\n"
    "</channel>\n");

  /* --- a private session bus --- */
  char sock[200], conf[300], addr[260];
  snprintf(sock, sizeof sock, "%s/bus", qt_tmp);
  snprintf(conf, sizeof conf, "%s/bus.conf", qt_tmp);
  snprintf(text, sizeof text,
    "<!DOCTYPE busconfig PUBLIC \"-//freedesktop//DTD D-Bus Bus Configuration 1.0//EN\"\n"
    " \"http://www.freedesktop.org/standards/dbus/1.0/busconfig.dtd\">\n"
    "<busconfig>\n  <type>session</type>\n  <listen>unix:path=%s</listen>\n  <allow_anonymous/>\n"
    "  <policy context=\"default\">\n    <allow send_destination=\"*\" eavesdrop=\"true\"/>\n"
    "    <allow eavesdrop=\"true\"/>\n    <allow own=\"*\"/>\n  </policy>\n</busconfig>\n", sock);
  write_file(conf, text);
  snprintf(log, sizeof log, "%s/daemons.log", qt_tmp);
  char carg[320]; snprintf(carg, sizeof carg, "--config-file=%s", conf);
  char *busv[] = { "dbus-daemon", carg, "--nofork", "--nopidfile", NULL };
  bus = spawn_bg(busv, log);
  int bus_up = bus > 0 && wait_path(sock, 100, &bus);
  QT_CHECK(bus_up, "private dbus-daemon listening on a UNIX socket");
  if (!bus_up) { show_log(log, 15); goto done; }
  snprintf(addr, sizeof addr, "unix:path=%s", sock);
  setenv("DBUS_SESSION_BUS_ADDRESS", addr, 1);

  char *xdv[] = { XFCONFD, NULL };
  xd = spawn_bg(xdv, log);
  /* xfconfd is up once it owns org.xfce.Xfconf and answers */
  const char *g = "";
  for (int i = 0; i < 40 && xd > 0; i++) {
    g = q("-c qnx-seed -p /greeting");
    if (*g) break;
    int st; if (waitpid(xd, &st, WNOHANG) == xd) { xd = -1; break; }
    usleep(250000);
  }
  QT_CHECK(strcmp(g, "hello from xml") == 0, "xfconfd serves the seeded channel: /greeting");
  if (!*g) { show_log(log, 15); goto done; }
  QT_CHECK(strcmp(q("-c qnx-seed -p /answer"), "42") == 0, "seeded int /answer = 42");
  QT_CHECK(strcmp(q("-c qnx-seed -p /nested/flag"), "true") == 0, "seeded nested bool /nested/flag = true");

  /* --- xfconf-query writes --- */
  rc = run("xfconf-query -c qnx-new -p /app/name -n -t string -s 'QNX Neutrino'", out, sizeof out, 30);
  QT_CHECK(rc == 0, "xfconf-query -n creates /app/name in a new channel");
  QT_CHECK(strcmp(q("-c qnx-new -p /app/name"), "QNX Neutrino") == 0, "string property created and read back");
  q("-c qnx-new -p /app/count -n -t int -s 7");
  q("-c qnx-new -p /app/count -s 8");
  QT_CHECK(strcmp(q("-c qnx-new -p /app/count"), "8") == 0, "int property created, then changed");
  q("-c qnx-new -p /app/list -n -t string -t string -s alpha -s beta");
  const char *lst = q("-c qnx-new -p /app/list");
  QT_CHECK(strstr(lst, "alpha") && strstr(lst, "beta") && strstr(lst, "2 items"), "array property (2 strings)");
  const char *props = q("-c qnx-new -l");
  QT_CHECK(strstr(props, "/app/name") && strstr(props, "/app/count") && strstr(props, "/app/list"), "-l lists the channel's properties");
  /* xfconfd lists channels from the XML files on disk, and writes a changed
   * channel only WRITE_TIMEOUT (5 s) after the last change: wait for
   * qnx-new.xml before expecting -l to list the new channel. */
  snprintf(path, sizeof path, "%s/config/xfce4/xfconf/xfce-perchannel-xml/qnx-new.xml", qt_tmp);
  int saved = wait_path(path, 150, NULL);
  printf("  info qnx-new.xml %s\n", saved ? "saved by xfconfd" : "not saved within 15 s");
  const char *chans = q("-l");
  { char flat[512]; size_t k = 0;
    for (const char *c = chans; *c && k < sizeof flat - 1; c++) flat[k++] = (*c == '\n') ? ' ' : *c;
    flat[k] = '\0'; printf("  info xfconf-query -l: %s\n", flat); }
  QT_CHECK(saved && strstr(chans, "qnx-seed") && strstr(chans, "qnx-new"), "-l lists both channels (after the delayed save)");
  q("-c qnx-seed -p /answer -r");
  rc = run("xfconf-query -c qnx-seed -p /answer", out, sizeof out, 30);
  QT_CHECK(rc != 0 && strstr(out, "does not exist"), "-r resets a property (now reported missing)");

  /* --- libxfconf in this process --- */
  GError *err = NULL;
  int inited = xfconf_init(&err);
  if (err) { printf("  info xfconf_init: %s\n", err->message); g_clear_error(&err); }
  QT_CHECK(inited, "xfconf_init on the private bus");
  if (inited) {
    XfconfChannel *ch = xfconf_channel_get("qnx-new");
    gchar *s = xfconf_channel_get_string(ch, "/app/name", NULL);
    QT_CHECK(s && strcmp(s, "QNX Neutrino") == 0, "libxfconf reads what xfconf-query wrote");
    g_free(s);
    QT_CHECK(xfconf_channel_get_int(ch, "/app/count", -1) == 8, "libxfconf get_int");
    QT_CHECK(xfconf_channel_set_double(ch, "/api/ratio", 0.25) && xfconf_channel_set_bool(ch, "/api/on", TRUE) &&
             xfconf_channel_set_uint(ch, "/api/big", 4000000000u), "libxfconf set double/bool/uint");
    QT_CHECK(strcmp(q("-c qnx-new -p /api/ratio"), "0.250000") == 0, "xfconf-query reads the double back");
    QT_CHECK(strcmp(q("-c qnx-new -p /api/on"), "true") == 0, "... the bool");
    QT_CHECK(strcmp(q("-c qnx-new -p /api/big"), "4000000000") == 0, "... the uint");
    const gchar *sl[] = { "one", "two", "three", NULL };
    xfconf_channel_set_string_list(ch, "/api/words", sl);
    gchar **got = xfconf_channel_get_string_list(ch, "/api/words");
    QT_CHECK(got && g_strv_length(got) == 3 && strcmp(got[2], "three") == 0, "string list round trip");
    g_strfreev(got);
    QT_CHECK(xfconf_channel_has_property(ch, "/api/on") && !xfconf_channel_has_property(ch, "/api/none"), "has_property");
    gchar **all = xfconf_list_channels();
    int seen = 0; for (int i = 0; all && all[i]; i++) if (!strcmp(all[i], "qnx-seed")) seen = 1;
    QT_CHECK(seen, "xfconf_list_channels includes qnx-seed");
    g_strfreev(all);

    xfconf_channel_set_string(ch, "/live/value", "before");
    g_signal_connect(ch, "property-changed::/live/value", G_CALLBACK(on_changed), NULL);
    q("-c qnx-new -p /live/value -s after");
    for (int i = 0; i < 100 && !changed; i++) { g_main_context_iteration(NULL, FALSE); usleep(50000); }
    printf("  info property-changed seen %d time(s), value \"%s\"\n", changed, changed_val);
    QT_CHECK(changed >= 1 && strcmp(changed_val, "after") == 0, "property-changed signal for xfconf-query's change");
    /* ch is a singleton owned by libxfconf (transfer none): no unref */
    g_signal_handlers_disconnect_by_func(ch, G_CALLBACK(on_changed), NULL);
    xfconf_shutdown();
  }

  /* --- xfconfd saves on exit --- */
  QT_CHECK(stop_bg(&xd), "xfconfd exits on SIGTERM");
  snprintf(path, sizeof path, "%s/config/xfce4/xfconf/xfce-perchannel-xml/qnx-new.xml", qt_tmp);
  QT_CHECK(file_has(path, "<channel name=\"qnx-new\"") && file_has(path, "value=\"QNX Neutrino\"") &&
           file_has(path, "name=\"count\" type=\"int\" value=\"8\""), "qnx-new.xml written with the new values");
  snprintf(path, sizeof path, "%s/config/xfce4/xfconf/xfce-perchannel-xml/qnx-seed.xml", qt_tmp);
  QT_CHECK(!file_has(path, "name=\"answer\"") && file_has(path, "hello from xml"), "qnx-seed.xml saved without the reset /answer");

done:
  if (xd > 0) stop_bg(&xd);
  if (bus > 0) stop_bg(&bus);
  scratch_down();
  QT_END();
}
