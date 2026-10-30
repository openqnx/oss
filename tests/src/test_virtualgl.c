#include "common.h"
#include <arpa/inet.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <turbojpeg.h>
/* The X11 path for VirtualGL, end to end on one QNX target, headless:
 * labwc (wlroots' headless backend) -> Xwayland, rootful, an X display on
 * it -> an X client draws a known colour and reads it back (XGetImage) ->
 * vglclient on that display -> this test plays VirtualGL's server side: it
 * speaks the VGL transport (version handshake, a JPEG tile, end of frame)
 * and sends a frame of a second colour for the window; vglclient must
 * decompress it and draw it there -- read back from the X server. That is
 * what vglrun on a Linux machine (RViz in an Ubuntu guest) does to show
 * frames on this display. */
#pragma pack(1)
typedef struct { unsigned int size, winid; unsigned short framew, frameh, width, height, x, y;
                 unsigned char qual, subsamp, flags, compress; unsigned short dpynum; } rrhdr;   /* 26 bytes */
typedef struct { char id[3]; unsigned char major, minor; } rrver;                                   /* 5 bytes */
#pragma pack()
static pid_t spawn(char *const a[]) { pid_t p = fork(); if (p == 0) { execvp(a[0], a); _exit(127); } return p; }
static int wait_file(const char *p, int tenths) { struct stat s; while (tenths-- > 0) { if (stat(p, &s) == 0) return 1; usleep(100000); } return 0; }
static int full(int fd, void *b, int n, int wr) { char *c = b; while (n > 0) { int r = wr ? (int)write(fd, c, n) : (int)read(fd, c, n); if (r <= 0) return 0; c += r; n -= r; } return 1; }
static int near(unsigned long px, Visual *v, int r, int g, int b, int tol) {
  int pr = (px & v->red_mask) >> __builtin_ctzl(v->red_mask), pg = (px & v->green_mask) >> __builtin_ctzl(v->green_mask),
      pb = (px & v->blue_mask) >> __builtin_ctzl(v->blue_mask);
  printf("  info pixel %02x%02x%02x (want %02x%02x%02x)\n", pr, pg, pb, r, g, b);
  return abs(pr - r) <= tol && abs(pg - g) <= tol && abs(pb - b) <= tol;
}
int main(void) {
  QT_BEGIN("virtualgl");
  int dn = 20 + getpid() % 40, port = 14242 + getpid() % 500;
  char rt[] = "/tmp/vgl-rt-XXXXXX"; mkdtemp(rt); chmod(rt, 0700);
  setenv("XDG_RUNTIME_DIR", rt, 1); setenv("WLR_BACKENDS", "headless", 1); setenv("WLR_RENDERER", "pixman", 1);
  char *lab[] = { "labwc", NULL }; pid_t lp = spawn(lab);
  char ws[300]; snprintf(ws, sizeof ws, "%s/wayland-0", rt);
  QT_CHECK(wait_file(ws, 150), "labwc up (Wayland compositor)");
  setenv("WAYLAND_DISPLAY", "wayland-0", 1);
  char dpy[16], dps[16]; snprintf(dpy, sizeof dpy, ":%d", dn); snprintf(dps, sizeof dps, "%d", port);
  char *xw[] = { "Xwayland", dpy, "-geometry", "320x240", NULL }; pid_t xp = spawn(xw);
  char xs[64]; snprintf(xs, sizeof xs, "/tmp/.X11-unix/X%d", dn);
  QT_CHECK(wait_file(xs, 150), "Xwayland up (an X display on labwc)");
  sleep(1);
  Display *d = XOpenDisplay(dpy);
  QT_CHECK(d != NULL, "XOpenDisplay");
  pid_t vp = -1;
  if (d) {
    int s = DefaultScreen(d); Visual *v = DefaultVisual(d, s);
    printf("  info X server: %s %d, screen %dx%d depth %d\n", ServerVendor(d), VendorRelease(d), DisplayWidth(d, s), DisplayHeight(d, s), DefaultDepth(d, s));
    Window w = XCreateSimpleWindow(d, RootWindow(d, s), 10, 10, 128, 96, 0, 0, 0);
    XMapWindow(d, w); XSync(d, False); usleep(300000);
    GC gc = XCreateGC(d, w, 0, NULL);
    XSetForeground(d, gc, (0x20UL << __builtin_ctzl(v->red_mask)) | (0xa0UL << __builtin_ctzl(v->green_mask)) | (0x60UL << __builtin_ctzl(v->blue_mask)));
    XFillRectangle(d, w, gc, 0, 0, 128, 96); XSync(d, False);
    XImage *im = XGetImage(d, w, 64, 48, 1, 1, AllPlanes, ZPixmap);
    QT_CHECK(im && near(XGetPixel(im, 0, 0), v, 0x20, 0xa0, 0x60, 0), "X11: drawn colour read back exactly");
    if (im) XDestroyImage(im);
    char *vc[] = { "vglclient", "-display", dpy, "-port", dps, NULL }; vp = spawn(vc);
    int fd = -1;
    for (int i = 0; i < 50 && fd < 0; i++) {
      int t = socket(AF_INET, SOCK_STREAM, 0); struct sockaddr_in a = { 0 };
      a.sin_family = AF_INET; a.sin_port = htons(port); a.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
      if (connect(t, (struct sockaddr *)&a, sizeof a) == 0) fd = t; else { close(t); usleep(100000); }
    }
    QT_CHECK(fd >= 0, "vglclient listening (VGL transport)");
    if (fd >= 0) {
      char z[24] = { 0 }; rrver cv, sv = { { 'V', 'G', 'L' }, 2, 1 };
      int ok = full(fd, z, 24, 1) && full(fd, &cv, 5, 0) && full(fd, &sv, 5, 1);
      printf("  info vglclient speaks VGL %d.%d\n", cv.major, cv.minor);
      QT_CHECK(ok && !memcmp(cv.id, "VGL", 3) && cv.major == 2, "version handshake (VGL 2.x)");
      unsigned char rgb[128 * 96 * 3];
      for (int i = 0; i < 128 * 96; i++) { rgb[i*3] = 0xc8; rgb[i*3+1] = 0x28; rgb[i*3+2] = 0x64; }
      tjhandle tj = tjInitCompress(); unsigned char *jpg = NULL; unsigned long jl = 0;
      tjCompress2(tj, rgb, 128, 0, 96, TJPF_RGB, &jpg, &jl, TJSAMP_444, 95, 0);
      rrhdr h = { (unsigned)jl, (unsigned)w, 128, 96, 128, 96, 0, 0, 95, 1, 0, 1, (unsigned short)dn };
      rrhdr e = h; e.size = 0; e.flags = 1;   /* RR_EOF */
      ok = full(fd, &h, sizeof h, 1) && full(fd, jpg, (int)jl, 1) && full(fd, &e, sizeof e, 1);
      QT_CHECK(ok, "frame sent: JPEG tile + end of frame");
      tjFree(jpg); tjDestroy(tj);
      usleep(1500000); XSync(d, False);
      im = XGetImage(d, w, 64, 48, 1, 1, AllPlanes, ZPixmap);
      QT_CHECK(im && near(XGetPixel(im, 0, 0), v, 0xc8, 0x28, 0x64, 6), "vglclient drew the frame in the window");
      if (im) XDestroyImage(im);
      close(fd);
    }
    XCloseDisplay(d);
  }
  if (vp > 0) { kill(vp, SIGTERM); waitpid(vp, NULL, 0); }
  kill(xp, SIGTERM); waitpid(xp, NULL, 0); kill(lp, SIGTERM); waitpid(lp, NULL, 0);
  QT_END();
}
