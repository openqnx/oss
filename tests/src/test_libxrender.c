/* libXrender against a real X server, headless on the target: labwc
 * (wlroots headless backend, pixman renderer) -> rootful Xwayland on a
 * private display. Checks: RENDER extension and version, standard picture
 * formats and their channel masks, XRenderParseColor, a solid
 * XRenderFillRectangle read back exactly with XGetImage, and a 50% alpha
 * ARGB32 source composited Over an opaque destination giving the blended
 * value (Porter-Duff arithmetic done by the server). */
#include "common.h"
#include <signal.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/extensions/Xrender.h>
#include <fcntl.h>
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
static pid_t spawn(char *const a[]) { return qx_spawn(a, NULL); }
static int wait_file(const char *p, int tenths) { struct stat s; while (tenths-- > 0) { if (stat(p, &s) == 0) return 1; usleep(100000); } return 0; }
static void stop(pid_t p) { qx_stop(p); }
static unsigned long px(Display *d, Pixmap p, int x, int y) {
  XImage *im = XGetImage(d, p, x, y, 1, 1, AllPlanes, ZPixmap);
  unsigned long v = im ? XGetPixel(im, 0, 0) : 0xdeadbeef;
  if (im) XDestroyImage(im);
  return v;
}
static int near(unsigned long v, unsigned long want, int tol) {
  for (int s = 0; s < 24; s += 8) { int a = (v >> s) & 0xff, b = (want >> s) & 0xff; if (abs(a - b) > tol) return 0; }
  return 1;
}
int main(void) {
  QT_BEGIN("libxrender");
  int dn = 20 + getpid() % 40;
  char rt[] = "/tmp/qnxport-xr-XXXXXX";
  if (!mkdtemp(rt)) { QT_CHECK(0, "mkdtemp runtime dir"); QT_END(); }
  chmod(rt, 0700);
  setenv("XDG_RUNTIME_DIR", rt, 1); setenv("WLR_BACKENDS", "headless", 1); setenv("WLR_RENDERER", "pixman", 1);
  unsetenv("WAYLAND_DISPLAY"); unsetenv("DISPLAY");
  char *lab[] = { "labwc", NULL }; pid_t lp = spawn(lab);
  char ws[300]; snprintf(ws, sizeof ws, "%s/wayland-0", rt);
  QT_CHECK(wait_file(ws, 150), "labwc up (headless Wayland compositor)");
  setenv("WAYLAND_DISPLAY", "wayland-0", 1);
  char dpy[16]; snprintf(dpy, sizeof dpy, ":%d", dn);
  char *xw[] = { "Xwayland", dpy, "-geometry", "320x240", NULL }; pid_t xp = spawn(xw);
  char xs[64]; snprintf(xs, sizeof xs, "/tmp/.X11-unix/X%d", dn);
  QT_CHECK(wait_file(xs, 150), "Xwayland up (X display on labwc)");
  Display *d = NULL;
  for (int i = 0; i < 50 && !d; i++) { d = XOpenDisplay(dpy); if (!d) usleep(100000); }
  QT_CHECK(d != NULL, "XOpenDisplay");
  if (d) {
    int ev = 0, er = 0, maj = 0, min = 0;
    QT_CHECK(XRenderQueryExtension(d, &ev, &er), "RENDER extension present");
    QT_CHECK(XRenderQueryVersion(d, &maj, &min) && (maj > 0 || min >= 10), "RENDER version >= 0.10");
    printf("  info RENDER %d.%d\n", maj, min);
    XRenderPictFormat *argb = XRenderFindStandardFormat(d, PictStandardARGB32);
    XRenderPictFormat *rgb = XRenderFindStandardFormat(d, PictStandardRGB24);
    XRenderPictFormat *a8 = XRenderFindStandardFormat(d, PictStandardA8);
    QT_CHECK(argb && argb->depth == 32 && argb->direct.alphaMask == 0xff && argb->direct.alpha == 24 && argb->direct.red == 16 &&
             argb->direct.redMask == 0xff, "ARGB32 format: depth 32, a@24 r@16");
    QT_CHECK(rgb && rgb->depth == 24 && rgb->direct.alphaMask == 0, "RGB24 format: no alpha");
    QT_CHECK(a8 && a8->depth == 8 && a8->direct.alphaMask == 0xff, "A8 format");
    XRenderColor c;
    /* XRenderParseColor returns colours premultiplied by their alpha (what
       RENDER's Over operator expects), so a translucent spec is scaled. */
    QT_CHECK(XRenderParseColor(d, "rgba:ff/80/00/ff", &c) && c.red == 0xffff && c.green == 0x8080 && c.blue == 0 && c.alpha == 0xffff,
             "XRenderParseColor rgba:ff/80/00/ff (opaque)");
    int pc = XRenderParseColor(d, "rgba:ff/80/00/40", &c);
    printf("  info rgba:ff/80/00/40 -> r %04x g %04x b %04x a %04x\n", c.red, c.green, c.blue, c.alpha);
    QT_CHECK(pc && c.alpha == 0x4040 && abs((int)c.red - 0x4040) <= 1 && abs((int)c.green - 0x2040) <= 1 && c.blue == 0,
             "XRenderParseColor rgba:ff/80/00/40 (premultiplied)");
    if (argb && rgb) {
      Window root = DefaultRootWindow(d);
      Pixmap dp = XCreatePixmap(d, root, 64, 64, 24), sp = XCreatePixmap(d, root, 64, 64, 32);
      Picture dst = XRenderCreatePicture(d, dp, rgb, 0, NULL), src = XRenderCreatePicture(d, sp, argb, 0, NULL);
      XRenderColor blue = { 0, 0, 0xffff, 0xffff }, red50 = { 0x8000, 0, 0, 0x8000 };   /* premultiplied */
      XRenderFillRectangle(d, PictOpSrc, dst, &blue, 0, 0, 64, 64);
      XSync(d, False);
      unsigned long v = px(d, dp, 10, 10);
      printf("  info filled pixel %06lx\n", v & 0xffffff);
      QT_CHECK((v & 0xffffff) == 0x0000ff, "XRenderFillRectangle: solid blue read back");
      XRenderFillRectangle(d, PictOpSrc, src, &red50, 0, 0, 64, 64);
      XRenderComposite(d, PictOpOver, src, None, dst, 0, 0, 0, 0, 16, 16, 32, 32);
      XSync(d, False);
      v = px(d, dp, 20, 20);
      unsigned long out = px(d, dp, 5, 5);
      printf("  info composited pixel %06lx (want ~80007f), outside %06lx\n", v & 0xffffff, out & 0xffffff);
      QT_CHECK(near(v, 0x80007f, 2), "Over: 50% red on blue -> (128,0,127)");
      QT_CHECK((out & 0xffffff) == 0x0000ff, "outside the composite rectangle unchanged");
      XRenderFreePicture(d, src); XRenderFreePicture(d, dst);
      XFreePixmap(d, sp); XFreePixmap(d, dp);
    }
    XCloseDisplay(d);
  }
  stop(xp); stop(lp);
  rmdir(rt);
  QT_END();
}
