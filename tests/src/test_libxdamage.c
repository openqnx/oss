/* libXdamage: damage objects on a window -- a raw DamageNotify for a filled rectangle, the accumulated damage region handed back by XDamageSubtract; against a headless labwc + Xwayland started by the test. */
#include "common.h"
#include <fcntl.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
/* ---- cleanup guard: every process this test starts is stopped on every
 * exit path -- return from main, exit(), a fatal signal (a crash in the
 * library under test) and the watchdog alarm. Each child leads a process
 * group of its own, so what it starts in turn (labwc may launch Xwayland or
 * helpers) is stopped with it. Left running, labwc keeps libdrm/libgbm busy
 * and breaks whatever runs on the target next. ---- */
#define QX_MAX_KIDS 8
static pid_t qx_kids[QX_MAX_KIDS];
static int qx_nkids;
static void qx_stop(pid_t p) {
  if (p <= 0) return;
  kill(-p, SIGTERM); kill(p, SIGTERM);
  int done = 0;
  for (int t = 0; t < 30 && !done; t++) { if (waitpid(p, NULL, WNOHANG) != 0) done = 1; else usleep(100000); }
  if (!done) { kill(-p, SIGKILL); kill(p, SIGKILL); waitpid(p, NULL, 0); }
  kill(-p, SIGKILL);   /* anything left in its group */
  for (int i = 0; i < qx_nkids; i++) if (qx_kids[i] == p) qx_kids[i] = -1;
}
static void qx_stop_all(void) {
  for (int i = qx_nkids - 1; i >= 0; i--) if (qx_kids[i] > 0) qx_stop(qx_kids[i]);
}
static void qx_on_signal(int sig) {
  static const char m[] = "  FAIL watchdog: test did not finish in time\n";
  if (sig == SIGALRM) (void)!write(1, m, sizeof m - 1);
  qx_stop_all();
  signal(sig, SIG_DFL); raise(sig);
}
static void qx_guard(unsigned seconds) {
  static int armed;
  if (armed) return;
  armed = 1;
  atexit(qx_stop_all);
  const int sigs[] = { SIGINT, SIGTERM, SIGHUP, SIGQUIT, SIGSEGV, SIGBUS, SIGABRT, SIGFPE, SIGILL, SIGALRM };
  for (unsigned i = 0; i < sizeof sigs / sizeof *sigs; i++) signal(sigs[i], qx_on_signal);
  alarm(seconds);
}
/* fork + exec in a new process group, recorded for qx_stop_all; log == NULL
   keeps the test's stdout/stderr */
static pid_t qx_spawn(char *const a[], const char *log) {
  qx_guard(240);
  pid_t p = fork();
  if (p == 0) {
    setpgid(0, 0);
    if (log) { int fd = open(log, O_WRONLY | O_CREAT | O_APPEND, 0644); if (fd >= 0) { dup2(fd, 1); dup2(fd, 2); close(fd); } }
    execvp(a[0], a); _exit(127);
  }
  if (p > 0) { setpgid(p, p); if (qx_nkids < QX_MAX_KIDS) qx_kids[qx_nkids++] = p; }
  return p;
}
/* ---- a private headless X server: labwc (wlroots headless backend) and a
 * rootful Xwayland on it, as tests/src/test_virtualgl.c does. ---- */
static pid_t xs_lab = -1, xs_xw = -1;
static char xs_rt[64], xs_dpy[16];
static pid_t xs_spawn(char *const a[], const char *log) { return qx_spawn(a, log); }
/* wait for a path to appear; give up early if the child that should create it died */
static int xs_wait(const char *path, int tenths, pid_t *child) {
  struct stat s;
  while (tenths-- > 0) {
    if (stat(path, &s) == 0) return 1;
    if (*child > 0 && waitpid(*child, NULL, WNOHANG) == *child) { *child = -1; return 0; }
    usleep(100000);
  }
  return 0;
}
static void xs_tail(void) {
  char cmd[160];
  snprintf(cmd, sizeof cmd, "tail -n 12 %s/server.log 2>/dev/null | sed 's/^/  info   | /'", xs_rt);
  (void)!system(cmd);
}
static Display *xs_start(void) {
  struct stat st;
  snprintf(xs_rt, sizeof xs_rt, "/tmp/qnxport-xs-XXXXXX");
  if (!mkdtemp(xs_rt)) { printf("  info mkdtemp failed\n"); return NULL; }
  chmod(xs_rt, 0700);
  setenv("XDG_RUNTIME_DIR", xs_rt, 1); setenv("WLR_BACKENDS", "headless", 1);
  setenv("WLR_RENDERER", "pixman", 1); unsetenv("WAYLAND_DISPLAY"); unsetenv("DISPLAY");
  char log[96]; snprintf(log, sizeof log, "%s/server.log", xs_rt);
  char *lab[] = { "labwc", NULL };
  xs_lab = xs_spawn(lab, log);
  char ws[96]; snprintf(ws, sizeof ws, "%s/wayland-0", xs_rt);
  if (!xs_wait(ws, 150, &xs_lab)) { printf("  info labwc did not come up (%s)\n", ws); xs_tail(); return NULL; }
  setenv("WAYLAND_DISPLAY", "wayland-0", 1);
  int dn; char xs[64], lk[64];
  for (dn = 20 + getpid() % 40; dn < 120; dn++) {
    snprintf(xs, sizeof xs, "/tmp/.X11-unix/X%d", dn); snprintf(lk, sizeof lk, "/tmp/.X%d-lock", dn);
    if (stat(xs, &st) != 0 && stat(lk, &st) != 0) break;
  }
  snprintf(xs_dpy, sizeof xs_dpy, ":%d", dn);
  char *xw[] = { "Xwayland", xs_dpy, "-geometry", "320x240", NULL };
  xs_xw = xs_spawn(xw, log);
  if (!xs_wait(xs, 150, &xs_xw)) { printf("  info Xwayland %s did not come up\n", xs_dpy); xs_tail(); return NULL; }
  Display *d = NULL;
  for (int i = 0; i < 50 && !d; i++) { d = XOpenDisplay(xs_dpy); if (!d) usleep(100000); }
  if (!d) { printf("  info XOpenDisplay(%s) failed\n", xs_dpy); xs_tail(); return NULL; }
  printf("  info X server %s: %s %d, screen %dx%d depth %d\n", xs_dpy, ServerVendor(d), VendorRelease(d),
         DisplayWidth(d, DefaultScreen(d)), DisplayHeight(d, DefaultScreen(d)), DefaultDepth(d, DefaultScreen(d)));
  return d;
}
static void xs_stop(Display *d) {
  if (d) XCloseDisplay(d);
  qx_stop(xs_xw); xs_xw = -1;
  qx_stop(xs_lab); xs_lab = -1;
  if (xs_rt[0]) { char cmd[96]; snprintf(cmd, sizeof cmd, "rm -rf %s", xs_rt); (void)!system(cmd); }
}
/* wait (bounded) for an event of the given type that matches pred */
__attribute__((unused)) static int xs_event(Display *d, int type, int (*pred)(XEvent *, void *), void *arg, XEvent *out) {
  for (int i = 0; i < 50; i++) {
    XSync(d, False);
    while (XPending(d)) {
      XEvent e; XNextEvent(d, &e);
      if (e.type == type && (!pred || pred(&e, arg))) { if (out) *out = e; return 1; }
    }
    usleep(100000);
  }
  return 0;
}
__attribute__((unused)) static Window xs_window(Display *d, int w, int h) {
  int s = DefaultScreen(d);
  Window win = XCreateSimpleWindow(d, RootWindow(d, s), 10, 10, w, h, 0, 0, 0);
  XMapWindow(d, win); XSync(d, False); usleep(200000);
  return win;
}
/* ---- end of server helper ---- */
#include <X11/extensions/Xdamage.h>
#include <X11/extensions/Xfixes.h>
static int dmg_ev;
static int is_fill(XEvent *e, void *a) {
  XDamageNotifyEvent *n = (XDamageNotifyEvent *)e; (void)a;
  printf("  info DamageNotify %dx%d+%d+%d\n", n->area.width, n->area.height, n->area.x, n->area.y);
  return n->area.x == 10 && n->area.y == 12 && n->area.width == 20 && n->area.height == 15;
}
int main(void) {
  QT_BEGIN("libxdamage");
  Display *d = xs_start();
  QT_CHECK(d != NULL, "headless X server (labwc + Xwayland) up");
  if (!d) { xs_stop(NULL); QT_END(); }
  int er, maj = 0, min = 0, fev, fer;
  QT_CHECK(XDamageQueryExtension(d, &dmg_ev, &er), "DAMAGE extension present");
  QT_CHECK(XDamageQueryVersion(d, &maj, &min) && maj >= 1, "DAMAGE version >= 1.0");
  printf("  info DAMAGE %d.%d\n", maj, min);
  int fmaj = 5, fmin = 0;
  XFixesQueryExtension(d, &fev, &fer); XFixesQueryVersion(d, &fmaj, &fmin);
  Window w = xs_window(d, 80, 60);
  GC gc = XCreateGC(d, w, 0, NULL);
  XSetForeground(d, gc, WhitePixel(d, DefaultScreen(d)));
  Damage raw = XDamageCreate(d, w, XDamageReportRawRectangles);
  Damage acc = XDamageCreate(d, w, XDamageReportNonEmpty);
  XSync(d, False);
  while (XPending(d)) { XEvent e; XNextEvent(d, &e); }       /* anything from mapping */
  XDamageSubtract(d, acc, None, None);
  XFillRectangle(d, w, gc, 10, 12, 20, 15);
  QT_CHECK(xs_event(d, dmg_ev + XDamageNotify, is_fill, NULL, NULL), "DamageNotify (raw) for the filled rectangle");
  XserverRegion parts = XFixesCreateRegion(d, NULL, 0);
  XDamageSubtract(d, acc, None, parts);
  int n = 0; XRectangle b;
  XRectangle *r = XFixesFetchRegionAndBounds(d, parts, &n, &b);
  printf("  info accumulated damage: %d rect(s), bounds %dx%d+%d+%d\n", n, b.width, b.height, b.x, b.y);
  QT_CHECK(r && n == 1 && b.x == 10 && b.y == 12 && b.width == 20 && b.height == 15,
           "XDamageSubtract hands back the damaged region");
  if (r) XFree(r);
  XFixesDestroyRegion(d, parts);
  /* after subtracting, nothing is left */
  parts = XFixesCreateRegion(d, NULL, 0);
  XDamageSubtract(d, acc, None, parts);
  r = XFixesFetchRegion(d, parts, &n);
  QT_CHECK(n == 0, "damage cleared after subtract");
  if (r) XFree(r);
  XFixesDestroyRegion(d, parts);
  XDamageDestroy(d, raw); XDamageDestroy(d, acc);
  XFreeGC(d, gc); XDestroyWindow(d, w);
  xs_stop(d);
  QT_END();
}
