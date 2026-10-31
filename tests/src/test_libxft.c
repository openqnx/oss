/* libXft: font-name parsing without a server (XftNameParse / Unparse), then
 * against a headless X server (labwc -> rootful Xwayland, as in
 * test_virtualgl): open DejaVu Sans from its file through
 * XftFontOpenPattern, check metrics, glyph coverage and text extents, draw
 * anti-aliased text with XftDrawStringUtf8 into a pixmap and read the
 * pixels back: background stays untouched, the glyphs' bounding box gets
 * ink, and an XftDrawRect fill is exact. */
#include "common.h"
#include <signal.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xft/Xft.h>
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
static const char *FONT = "/usr/local/share/fonts/font-dejavu/DejaVuSans.ttf";
static pid_t spawn(char *const a[]) { return qx_spawn(a, NULL); }
static int wait_file(const char *p, int tenths) { struct stat s; while (tenths-- > 0) { if (stat(p, &s) == 0) return 1; usleep(100000); } return 0; }
static void stop(pid_t p) { qx_stop(p); }
int main(void) {
  QT_BEGIN("libxft");
  printf("  info Xft %d\n", XftGetVersion());
  QT_CHECK(XftGetVersion() == XftVersion, "XftGetVersion matches the header");
  FcPattern *np = XftNameParse("DejaVu Sans-12:bold:pixelsize=20");
  char *fam = NULL; double ps = 0; int wt = 0;
  QT_CHECK(np && FcPatternGetString(np, FC_FAMILY, 0, (FcChar8 **)&fam) == FcResultMatch && strcmp(fam, "DejaVu Sans") == 0,
           "XftNameParse: family");
  QT_CHECK(np && FcPatternGetDouble(np, FC_PIXEL_SIZE, 0, &ps) == FcResultMatch && ps == 20 &&
           FcPatternGetInteger(np, FC_WEIGHT, 0, &wt) == FcResultMatch && wt == FC_WEIGHT_BOLD, "XftNameParse: pixel size and weight");
  char nb[256] = "";
  QT_CHECK(np && XftNameUnparse(np, nb, sizeof nb) && strstr(nb, "DejaVu Sans") && strstr(nb, "pixelsize=20"), "XftNameUnparse");
  printf("  info unparsed: %s\n", nb);
  if (np) FcPatternDestroy(np);
  struct stat fst;
  QT_CHECK(stat(FONT, &fst) == 0, "DejaVuSans.ttf installed");

  int dn = 20 + getpid() % 40;
  char rt[] = "/tmp/qnxport-xft-XXXXXX";
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
    int s = DefaultScreen(d);
    FcPattern *p = FcPatternBuild(NULL, FC_FILE, FcTypeString, FONT, FC_INDEX, FcTypeInteger, 0,
                                  FC_PIXEL_SIZE, FcTypeDouble, 24.0, FC_ANTIALIAS, FcTypeBool, FcTrue, (char *)0);
    XftFont *f = p ? XftFontOpenPattern(d, p) : NULL;   /* takes ownership of p on success */
    QT_CHECK(f != NULL, "XftFontOpenPattern(DejaVuSans.ttf, 24px)");
    if (!f && p) FcPatternDestroy(p);
    if (f) {
      printf("  info ascent %d descent %d height %d max_advance %d\n", f->ascent, f->descent, f->height, f->max_advance_width);
      QT_CHECK(f->ascent >= 18 && f->ascent <= 26 && f->descent >= 3 && f->descent <= 9, "ascent/descent plausible for 24px");
      QT_CHECK(XftCharExists(d, f, 'A') && XftCharIndex(d, f, 'A') != 0 && XftCharExists(d, f, 0x20ac), "glyphs for A and the euro sign");
      XGlyphInfo gi, gw;
      XftTextExtents8(d, f, (const FcChar8 *)"i", 1, &gi);
      XftTextExtents8(d, f, (const FcChar8 *)"W", 1, &gw);
      XGlyphInfo gt; XftTextExtentsUtf8(d, f, (const FcChar8 *)"QNX", 3, &gt);
      printf("  info advance i=%d W=%d QNX=%d\n", gi.xOff, gw.xOff, gt.xOff);
      QT_CHECK(gi.xOff > 0 && gw.xOff > 2 * gi.xOff && gt.xOff > gw.xOff, "text extents: W wider than i, QNX wider than W");

      Visual *v = DefaultVisual(d, s); Colormap cm = DefaultColormap(d, s);
      Pixmap pm = XCreatePixmap(d, RootWindow(d, s), 120, 40, DefaultDepth(d, s));
      XftDraw *dr = XftDrawCreate(d, pm, v, cm);
      QT_CHECK(dr != NULL, "XftDrawCreate on a pixmap");
      XRenderColor kc = { 0, 0, 0, 0xffff }, wc = { 0xffff, 0xffff, 0xffff, 0xffff }, gc = { 0x2000, 0xa000, 0x6000, 0xffff };
      XftColor black, white, green;
      QT_CHECK(XftColorAllocValue(d, v, cm, &kc, &black) && XftColorAllocValue(d, v, cm, &wc, &white) &&
               XftColorAllocValue(d, v, cm, &gc, &green), "XftColorAllocValue");
      if (dr) {
        XftDrawRect(dr, &black, 0, 0, 120, 40);
        XftDrawRect(dr, &green, 100, 0, 20, 40);
        XftDrawStringUtf8(dr, &white, f, 2, f->ascent + 2, (const FcChar8 *)"QNX", 3);
        XSync(d, False);
        XImage *im = XGetImage(d, pm, 0, 0, 120, 40, AllPlanes, ZPixmap);
        QT_CHECK(im != NULL, "XGetImage of the pixmap");
        if (im) {
          int lit = 0, full = 0, stray = 0;
          for (int y = 0; y < 40; y++)
            for (int x = 0; x < 100; x++) {
              unsigned long q = XGetPixel(im, x, y) & 0xffffff;
              int inbox = x < gt.xOff + 6 && y < f->ascent + f->descent + 4;
              if (q) { if (inbox) lit++; else stray++; }
              if (q == 0xffffff) full++;
            }
          unsigned long g = XGetPixel(im, 110, 20) & 0xffffff;
          printf("  info ink: %d lit pixels (%d full white), %d outside the text box; rect %06lx\n", lit, full, stray, g);
          QT_CHECK(lit > 60 && full > 20, "text drawn: glyph pixels lit inside the text box");
          QT_CHECK(stray == 0, "no ink outside the text box");
          QT_CHECK(g == ((0x20UL << 16) | (0xa0UL << 8) | 0x60UL) || v->red_mask != 0xff0000, "XftDrawRect colour exact");
          XDestroyImage(im);
        }
        XftDrawDestroy(dr);
      }
      XftColorFree(d, v, cm, &black); XftColorFree(d, v, cm, &white); XftColorFree(d, v, cm, &green);
      XFreePixmap(d, pm);
      XftFontClose(d, f);
    }
    XCloseDisplay(d);
  }
  stop(xp); stop(lp);
  rmdir(rt);
  QT_END();
}
