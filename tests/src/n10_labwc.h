/* Headless Wayland compositor for the GTK 4 tests of group n10-libs-svc
 * (test_libadwaita, test_libpanel): labwc on wlroots' headless backend with
 * the pixman renderer, a private XDG_RUNTIME_DIR, and GTK told to draw with
 * cairo (QNX's only EGL is Screen's, so GTK's GL renderers cannot work on a
 * Wayland display here). Every process started is stopped on every exit
 * path: return from main, exit(), a fatal signal and the watchdog alarm --
 * the pattern of test_libxfce4windowing.c. */
#ifndef N10_LABWC_H
#define N10_LABWC_H
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>

static pid_t n10_labwc_pid = -1;
static char n10_rt[64], n10_sock[128];

static void n10_stop(void) {
  pid_t p = n10_labwc_pid;
  if (p <= 0) return;
  n10_labwc_pid = -1;
  kill(-p, SIGTERM); kill(p, SIGTERM);
  int done = 0;
  for (int t = 0; t < 30 && !done; t++) { if (waitpid(p, NULL, WNOHANG) != 0) done = 1; else usleep(100000); }
  if (!done) { kill(-p, SIGKILL); kill(p, SIGKILL); waitpid(p, NULL, 0); }
  kill(-p, SIGKILL);
  unlink(n10_sock);
  char lk[140]; snprintf(lk, sizeof lk, "%s.lock", n10_sock); unlink(lk);
  rmdir(n10_rt);
}
static void n10_on_signal(int sig) {
  static const char m[] = "  FAIL watchdog: test did not finish in time\n";
  if (sig == SIGALRM) (void)!write(1, m, sizeof m - 1);
  n10_stop();
  signal(sig, SIG_DFL); raise(sig);
}
/* Starts labwc and waits for its socket; returns 1 when WAYLAND_DISPLAY is
   set and usable. */
static int n10_start_labwc(unsigned watchdog_seconds) {
  atexit(n10_stop);
  const int sigs[] = { SIGINT, SIGTERM, SIGHUP, SIGQUIT, SIGSEGV, SIGBUS, SIGABRT, SIGFPE, SIGILL, SIGALRM };
  for (unsigned i = 0; i < sizeof sigs / sizeof *sigs; i++) signal(sigs[i], n10_on_signal);
  alarm(watchdog_seconds);
  snprintf(n10_rt, sizeof n10_rt, "/tmp/qnxport-n10wl-%d", (int)getpid());
  if (mkdir(n10_rt, 0700) != 0) return 0;
  setenv("XDG_RUNTIME_DIR", n10_rt, 1);
  setenv("WLR_BACKENDS", "headless", 1); setenv("WLR_RENDERER", "pixman", 1);
  unsetenv("WAYLAND_DISPLAY"); unsetenv("DISPLAY");
  pid_t p = fork();
  if (p == 0) {
    setpgid(0, 0);
    char log[128]; snprintf(log, sizeof log, "%s/labwc.log", n10_rt);
    int fd = open(log, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd >= 0) { dup2(fd, 1); dup2(fd, 2); close(fd); }
    execlp("labwc", "labwc", (char *)NULL);
    _exit(127);
  }
  if (p < 0) return 0;
  setpgid(p, p);
  n10_labwc_pid = p;
  snprintf(n10_sock, sizeof n10_sock, "%s/wayland-0", n10_rt);
  struct stat sb;
  for (int i = 0; i < 150; i++) {
    if (stat(n10_sock, &sb) == 0) {
      setenv("WAYLAND_DISPLAY", "wayland-0", 1);
      setenv("GDK_BACKEND", "wayland", 1);
      setenv("GSK_RENDERER", "cairo", 1);
      setenv("GDK_DISABLE", "gl,vulkan", 1);
      setenv("GTK_A11Y", "none", 1);
      return 1;
    }
    if (waitpid(p, NULL, WNOHANG) == p) { n10_labwc_pid = -1; break; }
    usleep(100000);
  }
  return 0;
}
#endif
