#include "common.h"
#include <errno.h>
#include <poll.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <time.h>
#include <unistd.h>
#include <unix.h>
/* htop and btop are full-screen programs: each runs here on a pseudo
 * terminal (openpty, 120x40) until its process list is up (at most 20 s:
 * emulated targets are slow to start), gets "q", and what it drew
 * is checked -- htop's QNX platform must list this target's processes
 * (sshd and io-sock are always running), btop's QNX collector must draw its
 * cpu, mem and proc boxes. Both must exit on "q". */
static int has(const char *b, size_t n, const char *s);
/* Runs until the screen shows `ready` (or 20 s), then sends "q". */
static size_t run(const char *prog, char *const argv[], const char *ready, char *buf, size_t cap, int *status) {
  /* openpty + the child's own session with the slave as its controlling
   * terminal and stdio. (QNX's forkpty() left the child writing to the
   * caller's terminal when the caller had none of its own -- under sshd.) */
  int m, s; struct winsize ws = { 40, 120, 0, 0 };
  if (openpty(&m, &s, NULL, NULL, &ws) != 0) return 0;
  pid_t p = fork();
  if (p == 0) {
    close(m); setsid(); ioctl(s, TIOCSCTTY, 0);
    dup2(s, 0); dup2(s, 1); dup2(s, 2); if (s > 2) close(s);
    setenv("TERM", "xterm-256color", 1); execvp(prog, argv); _exit(127);
  }
  close(s);
  if (p < 0) { close(m); return 0; }
  size_t n = 0; time_t start = time(NULL), end = start + 20; int quit_sent = 0;
  for (;;) {
    struct pollfd pf = { m, POLLIN, 0 };
    if (poll(&pf, 1, 200) > 0) {
      ssize_t r = read(m, buf + n, cap - 1 - n);
      if (r <= 0) break;
      n += (size_t)r; if (n >= cap - 1) n = cap / 2;   /* keep the latest screens */
    }
    if (!quit_sent && (time(NULL) >= end || (time(NULL) >= start + 2 && has(buf, n, ready)))) {
      if (time(NULL) < end) usleep(500000);   /* let it finish the screen */
      (void)!write(m, "q", 1); quit_sent = 1; end = time(NULL) + 5;
    }
    else if (quit_sent && time(NULL) >= end) break;
  }
  buf[n] = 0;
  int w = 0;
  for (int i = 0; i < 30 && w == 0; i++) { w = waitpid(p, status, WNOHANG); if (!w) usleep(100000); }
  if (w == 0) { kill(p, SIGKILL); waitpid(p, status, 0); *status = -1; }
  close(m);
  return n;
}
static int has(const char *b, size_t n, const char *s) { return memmem(b, n, s, strlen(s)) != NULL; }
int main(void) {
  QT_BEGIN("monitors");
  static char buf[1 << 20]; int st;
  char *h[] = { "htop", "-d", "10", NULL };
  size_t n = run("htop", h, "io-sock", buf, sizeof buf, &st);
  printf("  info htop drew %zu bytes\n", n);
  if (n < 1000) { for (size_t i = 0; i < n; i++) if (buf[i] < 32 && buf[i] != '\n') buf[i] = '.'; printf("  info htop said: %.*s\n", (int)n, buf); }
  QT_CHECK(n > 1000, "htop drew a screen");
  QT_CHECK(has(buf, n, "PID") && has(buf, n, "Mem"), "htop: header and meters");
  QT_CHECK(has(buf, n, "sshd"), "htop: lists sshd (QNX process table)");
  QT_CHECK(has(buf, n, "io-sock"), "htop: lists io-sock");
  QT_CHECK(st == 0, "htop exits on q");
  char *b[] = { "btop", NULL };
  n = run("btop", b, "sshd", buf, sizeof buf, &st);
  printf("  info btop drew %zu bytes\n", n);
  if (n < 1000) { for (size_t i = 0; i < n; i++) if (buf[i] < 32 && buf[i] != '\n') buf[i] = '.'; printf("  info btop said: %.*s\n", (int)n, buf); }
  QT_CHECK(n > 1000, "btop drew a screen");
  QT_CHECK(has(buf, n, "cpu") && has(buf, n, "mem") && has(buf, n, "proc"), "btop: cpu, mem, proc boxes");
  QT_CHECK(has(buf, n, "sshd") || has(buf, n, "io-sock"), "btop: lists this target's processes");
  QT_CHECK(st == 0, "btop exits on q");
  QT_END();
}
