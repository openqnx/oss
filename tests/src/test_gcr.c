/* gcr: GNOME's certificate library, used for real.  gcr-base parses a PEM
 * bundle (a self-signed X.509 certificate and its PKCS#8 RSA key, generated
 * once with openssl and embedded below), and the certificate's subject,
 * issuer, serial number, key size and SHA-1/SHA-256 fingerprints must equal
 * what openssl reported for it.  gcr-viewer prints its version from its
 * option callback, before it would open a display.
 *   openssl x509: subject=C=CA, O=QNX Port Tests, OU=g4, CN=qnx-port-test.example
 *   serial=1234ABCD, RSA 2048, DER 891 bytes
 *   sha1   B30AB5EA146D1BDB1D32D51F8A053D345A952264
 *   sha256 EA717CC6762296D3E17EB5E4AAB8C74FC821EA2246E31E40F65DBE1D40E45901 */
#define GCR_API_SUBJECT_TO_CHANGE 1
#include "common.h"
#include <ctype.h>
#include <glib.h>
#include <gcr/gcr-base.h>
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
static const char CERT_PEM[] =
  "-----BEGIN CERTIFICATE-----\n"
  "MIIDdzCCAl+gAwIBAgIEEjSrzTANBgkqhkiG9w0BAQsFADBTMQswCQYDVQQGEwJD\n"
  "QTEXMBUGA1UECgwOUU5YIFBvcnQgVGVzdHMxCzAJBgNVBAsMAmc0MR4wHAYDVQQD\n"
  "DBVxbngtcG9ydC10ZXN0LmV4YW1wbGUwHhcNMjYxMDMxMTA0NDM2WhcNNDYxMDI2\n"
  "MTA0NDM2WjBTMQswCQYDVQQGEwJDQTEXMBUGA1UECgwOUU5YIFBvcnQgVGVzdHMx\n"
  "CzAJBgNVBAsMAmc0MR4wHAYDVQQDDBVxbngtcG9ydC10ZXN0LmV4YW1wbGUwggEi\n"
  "MA0GCSqGSIb3DQEBAQUAA4IBDwAwggEKAoIBAQCfbpOO7EpMpcz+g2JMnqslKJ0i\n"
  "NlRAwBGRJ+i16cxeGODzlXCeaIUSpydYw1aK/7w4XsJ4W7w8XQbv56AN5bhqxoY8\n"
  "8N7ZoL0+ldJvz7B7lG7IAfGwvr3dEmKHLtJgmoagNMflR31T05JSYvRIh/VRB2IY\n"
  "AVNLB7QrX5K+7rc+2mIGvdrl1EgUu3RAXnai/XkcCCQGjhGsSexw6vXnPcwWGP+G\n"
  "jXZGKuiQzTcbExCZGn7hsT1q2v+tA3eX0cjPy/QRRHELsITgIR0cavrlXylsVRw0\n"
  "m3iVkyO8rnGMjRnvpoxigsLvgOXd8aJKq0go1nPmN0FXzk3AuBT+Gn+XOJMLAgMB\n"
  "AAGjUzBRMB0GA1UdDgQWBBTZCkprqQotBH7U4/0U/T90RUat/jAfBgNVHSMEGDAW\n"
  "gBTZCkprqQotBH7U4/0U/T90RUat/jAPBgNVHRMBAf8EBTADAQH/MA0GCSqGSIb3\n"
  "DQEBCwUAA4IBAQBOjLYwKx/YVK/9OgBtyTKtRczz5a7pQnWopWTCn+M6SnjdZhm2\n"
  "fNhaLl6RL7rRpHgMFDNj8VdNkZSpLaF9hHnTBFzFkONWIk1KhrRr4llT4rbYkR58\n"
  "5xUA896GUwaKMJ1Ru2fC46sTdq+NvR27dMyH0zisagsPx/30COKSQ9mbrZe9H+L4\n"
  "WflRP0AJN6XeesPYevbkfqtkAtdnthkdhM53wfkaC05QO/ITDQ20OtOJCt6QIHQx\n"
  "seSaxfuT0FczV6YosW3bmwuTmmeKhVrxtajbSL/nM20lv/OS9dsYZWjYegKoQcd2\n"
  "WVXgjcvrzsHSBoDiZykYm2gSqJGFowsKWRq1\n"
  "-----END CERTIFICATE-----\n"
  ;
static const char KEY_PEM[] =
  "-----BEGIN PRIVATE KEY-----\n"
  "MIIEvAIBADANBgkqhkiG9w0BAQEFAASCBKYwggSiAgEAAoIBAQCfbpOO7EpMpcz+\n"
  "g2JMnqslKJ0iNlRAwBGRJ+i16cxeGODzlXCeaIUSpydYw1aK/7w4XsJ4W7w8XQbv\n"
  "56AN5bhqxoY88N7ZoL0+ldJvz7B7lG7IAfGwvr3dEmKHLtJgmoagNMflR31T05JS\n"
  "YvRIh/VRB2IYAVNLB7QrX5K+7rc+2mIGvdrl1EgUu3RAXnai/XkcCCQGjhGsSexw\n"
  "6vXnPcwWGP+GjXZGKuiQzTcbExCZGn7hsT1q2v+tA3eX0cjPy/QRRHELsITgIR0c\n"
  "avrlXylsVRw0m3iVkyO8rnGMjRnvpoxigsLvgOXd8aJKq0go1nPmN0FXzk3AuBT+\n"
  "Gn+XOJMLAgMBAAECggEARafRDjuMRqqVwojydJlKGuUAqgbkjpmb0ziFgp0QLfhe\n"
  "p+2dk4gEw96F4VEOyZHr0RENk/o4ZoMaWcxv8EJybSwpUAzIwHpq1CIvG6y45XU3\n"
  "ZWj8CIV9fE3hAEDjp1M8CFinETRHvE0Oe7JCGkJUWr+ICwbhNkG6ZMzP112CcBHW\n"
  "p41A5PlKz8Z18WZqq6jVZWFY0KjtbOi/lI8ueODho0cpMayxO4gZDbw1s6ofz6vf\n"
  "FhofaauR8dYDBBxlS0dCM6v/ED2KIl5ry6FBLOAmX2B+vfhmmlif7TR9IQCjxNvi\n"
  "kqAResD+EEW5sLt738r1xWwkXPUMZxtzYzfmtZO6yQKBgQDXW8oCZP4rvviLksEq\n"
  "5Y+EgfRixeFyGWvE6uqVTUzTjAiHIJNqn0nARnIrlqXcs7qdjGZL/V2aQ5F/v0Yq\n"
  "ufq/rzQKzNO7QBgRyNFPbjUV0w00/Ih4+IiXhZ1xJi5RnAqpFgMcsU2nlS4YNetK\n"
  "dtBuO9KHCsWuTmYQTsOfgezTTQKBgQC9hOkYT621nURUGjJImeBsVZEOuVmEueR9\n"
  "Lo3mYYfyyfvCZLC25pICP8bCT8tF2c7dGrzuzQxsqwsyHI3JV+k98PraxuY1MSEn\n"
  "6yd4t3soaHc1IiQtbZKkH8fxoBpFkF1DXMhtD0Qbbk7PO8idBpYh/IYYsj9/ckAL\n"
  "Akw+1TIjtwKBgGfEaI/Czd2SRPwrmDvtJLv6SiEVq81ohk75ylJqTqxX+vvxLFbh\n"
  "rY2h4/0wt3aiezbSoa/XwzRDfmYJaevIdRPLfc3+Z77/4RkgpAob1iRP7zsquy9e\n"
  "waIQ1Y3hkcvs3g4e9xzqKbqn3O/5ewgEO+YBmS1MeXfFetQ8v3GqDsy5AoGACXZB\n"
  "TP1tZsvbRXepWwaTtdTv0RRf68h8J+PTIdVsvCppb2Mnj3ojgAk/GyRwuaQON+vF\n"
  "0wZZwHMps+7JBWxmMLl/TLzxcWZqiVusNNK0S2gry3NQ6xO1ZoxPpzifFyoF/f77\n"
  "4rYIaHYxaRdLBMv3YusApur/UJWbrPEIbXqnbjECgYBx6yvNLGwdFqjoAJsWsfHO\n"
  "ViIQvqKqtZ9FengVez8lQary4ThObigkCxKOdy1vBXqsTLHjkLQicyqcygtRWA2r\n"
  "CGISNf+4Xhk6RikTKv5/XPnDBoxdteiGOn0q6+NiHgKK45sg9BvqXLD98E5A0Wvr\n"
  "4clqoEG5yiqHPzt+Wl+HQA==\n"
  "-----END PRIVATE KEY-----\n"
  ;

/* "EA 71 7C ..." / "ea:71:..." -> "EA717C..." */
static char *hex_norm(const char *s) {
  static char b[160]; size_t j = 0;
  for (; s && *s && j < sizeof b - 1; s++) if (isxdigit((unsigned char)*s)) b[j++] = (char)toupper((unsigned char)*s);
  b[j] = '\0'; return b;
}
static const char *nz(const char *s) { return s ? s : "(null)"; }

static int n_parsed = 0, saw_cert = 0, saw_key = 0;
static char cert_label[128];
static void on_parsed(GcrParser *p, gpointer data) {
  (void)data;
  const char *desc = gcr_parser_get_parsed_description(p);
  const char *label = gcr_parser_get_parsed_label(p);
  n_parsed++;
  printf("  info parsed #%d: %s, label \"%s\", format %d\n", n_parsed, nz(desc), nz(label),
         (int)gcr_parser_get_parsed_format(p));
  if (desc && strcmp(desc, "Certificate") == 0) {
    saw_cert = 1; g_strlcpy(cert_label, nz(label), sizeof cert_label);
  }
  if (desc && strcmp(desc, "Private Key") == 0) saw_key = 1;
}

int main(void) {
  QT_BEGIN("gcr");
  static char out[8192];

  /* --- the parser: one PEM buffer, two objects --- */
  GcrParser *parser = gcr_parser_new();
  QT_CHECK(parser != NULL, "gcr_parser_new");
  gcr_parser_format_enable(parser, GCR_FORMAT_ALL);
  g_signal_connect(parser, "parsed", G_CALLBACK(on_parsed), NULL);
  GString *bundle = g_string_new(CERT_PEM);
  g_string_append(bundle, KEY_PEM);
  GError *err = NULL;
  gboolean ok = gcr_parser_parse_data(parser, (const guchar *)bundle->str, bundle->len, &err);
  if (err) { printf("  info parse error: %s\n", err->message); g_clear_error(&err); }
  QT_CHECK(ok, "gcr_parser_parse_data on a PEM certificate + key bundle");
  QT_CHECK(n_parsed == 2, "two objects parsed");
  QT_CHECK(saw_cert, "one is a Certificate");
  QT_CHECK(saw_key, "one is a Private Key");
  QT_CHECK(strcmp(cert_label, "qnx-port-test.example") == 0, "certificate label is its subject CN");
  g_string_free(bundle, TRUE);
  g_object_unref(parser);

  /* --- the certificate itself, from its DER bytes --- */
  const char *b = strstr(CERT_PEM, "-----\n") + 6;
  const char *e = strstr(CERT_PEM, "-----END");
  gchar *b64 = g_strndup(b, (gsize)(e - b));
  gsize der_len = 0;
  guchar *der = g_base64_decode(b64, &der_len);
  g_free(b64);
  printf("  info DER certificate: %u bytes\n", (unsigned)der_len);
  QT_CHECK(der_len == 891, "PEM body decodes to the 891-byte DER certificate");
  GcrCertificate *cert = gcr_simple_certificate_new(der, der_len);
  QT_CHECK(cert != NULL, "gcr_simple_certificate_new");
  if (cert) {
    gchar *cn = gcr_certificate_get_subject_cn(cert);
    gchar *icn = gcr_certificate_get_issuer_cn(cert);
    gchar *org = gcr_certificate_get_subject_part(cert, "O");
    gchar *ou = gcr_certificate_get_subject_part(cert, "OU");
    gchar *c = gcr_certificate_get_subject_part(cert, "C");
    gchar *dn = gcr_certificate_get_subject_dn(cert);
    printf("  info subject DN: %s\n", nz(dn));
    QT_CHECK(cn && strcmp(cn, "qnx-port-test.example") == 0, "subject CN");
    QT_CHECK(icn && strcmp(icn, "qnx-port-test.example") == 0, "issuer CN (self-signed)");
    QT_CHECK(org && strcmp(org, "QNX Port Tests") == 0, "subject O");
    QT_CHECK(ou && strcmp(ou, "g4") == 0, "subject OU");
    QT_CHECK(c && strcmp(c, "CA") == 0, "subject C");
    QT_CHECK(gcr_certificate_is_issuer(cert, cert), "certificate is its own issuer");
    gchar *serial = gcr_certificate_get_serial_number_hex(cert);
    printf("  info serial: %s\n", nz(serial));
    QT_CHECK(serial && strcmp(hex_norm(serial), "1234ABCD") == 0, "serial number 1234ABCD");
    guint bits = gcr_certificate_get_key_size(cert);
    printf("  info key size: %u bits\n", bits);
    QT_CHECK(bits == 2048, "RSA key size 2048");
    gchar *sha1 = gcr_certificate_get_fingerprint_hex(cert, G_CHECKSUM_SHA1);
    gchar *sha256 = gcr_certificate_get_fingerprint_hex(cert, G_CHECKSUM_SHA256);
    printf("  info sha256: %s\n", nz(sha256));
    QT_CHECK(sha1 && strcmp(hex_norm(sha1), "B30AB5EA146D1BDB1D32D51F8A053D345A952264") == 0,
             "SHA-1 fingerprint matches openssl");
    QT_CHECK(sha256 && strcmp(hex_norm(sha256),
             "EA717CC6762296D3E17EB5E4AAB8C74FC821EA2246E31E40F65DBE1D40E45901") == 0,
             "SHA-256 fingerprint matches openssl");
    GDate *exp = gcr_certificate_get_expiry_date(cert);
    if (exp) printf("  info expires %04d-%02d-%02d\n", g_date_get_year(exp), g_date_get_month(exp), g_date_get_day(exp));
    QT_CHECK(exp && g_date_get_year(exp) == 2046, "expiry year 2046 (20-year certificate)");
    if (exp) g_date_free(exp);
    g_free(cn); g_free(icn); g_free(org); g_free(ou); g_free(c); g_free(dn);
    g_free(serial); g_free(sha1); g_free(sha256);
    g_object_unref(cert);
  }
  g_free(der);

  /* --- gcr-viewer --version (printed while options are parsed) --- */
  if (scratch_up("gcr")) {
    int rc = run("gcr-viewer --version", out, sizeof out, 30);
    printf("  info gcr-viewer --version -> %d: %s\n", rc, line1(out));
    QT_CHECK(rc == 0 && strstr(out, "GCR Certificate and Key Viewer") && strstr(out, "3.41.2"),
             "gcr-viewer --version: GCR Certificate and Key Viewer -- 3.41.2");
    scratch_down();
  }
  QT_END();
}
