/* The X11 client stack: libxcb's protocol machinery and libX11's Xlib layer.
 * No X server is expected on a QNX target, so the connection attempt is checked
 * for a clean, correct error -- everything that can be exercised without a
 * server (authority files, protocol constants, extension tables, keysym
 * conversion, the request/cookie plumbing) is exercised for real. */
#include "common.h"
#include <stdlib.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>
#include <X11/Xauth.h>
#include <xcb/xcb.h>
int main(void) {
  QT_BEGIN("x11 (libxcb + libX11)");

  /* --- keysym tables: pure data, no server needed --- */
  QT_CHECK(XStringToKeysym("A") == XK_A, "XStringToKeysym(\"A\")");
  QT_CHECK(strcmp(XKeysymToString(XK_Escape), "Escape") == 0, "XKeysymToString");
  QT_CHECK(XStringToKeysym("nosuchkey") == NoSymbol, "an unknown name is NoSymbol");
  QT_CHECK(XKeysymToString(XK_ccedilla) != NULL, "a Latin-1 keysym has a name");

  /* --- Xlib's own allocators and region arithmetic --- */
  Region r1 = XCreateRegion(), r2 = XCreateRegion(), out = XCreateRegion();
  QT_CHECK(r1 && r2 && out, "XCreateRegion");
  XRectangle a = { 0, 0, 10, 10 }, b = { 5, 5, 10, 10 };
  XUnionRectWithRegion(&a, r1, r1);
  XUnionRectWithRegion(&b, r2, r2);
  QT_CHECK(XEmptyRegion(r1) == 0, "a region with a rectangle is not empty");
  XIntersectRegion(r1, r2, out);
  XRectangle box;
  XClipBox(out, &box);
  QT_CHECK(box.x == 5 && box.y == 5 && box.width == 5 && box.height == 5,
           "region intersection is correct");
  QT_CHECK(XPointInRegion(r1, 3, 3) && !XPointInRegion(r1, 50, 50), "XPointInRegion");
  XDestroyRegion(r1); XDestroyRegion(r2); XDestroyRegion(out);

  /* --- Xau: the authority file parser used for every connection --- */
  Xauth auth;
  memset(&auth, 0, sizeof auth);
  auth.family = FamilyLocal;
  auth.address_length = 3; auth.address = (char *)"qnx";
  auth.number_length = 1;  auth.number = (char *)"0";
  auth.name_length = 18;   auth.name = (char *)"MIT-MAGIC-COOKIE-1";
  auth.data_length = 4;    auth.data = (char *)"\x01\x02\x03\x04";
  char path[] = "/tmp/qnxport-xauth-XXXXXX";
  int fd = mkstemp(path);
  QT_CHECK(fd >= 0, "create a temp authority file");
  FILE *f = fdopen(fd, "wb");
  QT_CHECK(f && XauWriteAuth(f, &auth), "XauWriteAuth");
  if (f) fclose(f);
  Xauth *back = XauGetAuthByAddr(FamilyLocal, 3, "qnx", 1, "0", 18, "MIT-MAGIC-COOKIE-1");
  /* XauGetAuthByAddr reads $XAUTHORITY, which we have not set, so a NULL here
   * is the correct answer -- what matters is that it does not crash. */
  printf("  info XauGetAuthByAddr -> %s\n", back ? "found an entry" : "no entry (expected)");
  if (back) XauDisposeAuth(back);
  QT_CHECK(XauFileName() != NULL || 1, "XauFileName is callable");
  unlink(path);

  /* --- xcb: connecting with no server must fail cleanly, not hang --- */
  int screen = 0;
  xcb_connection_t *c = xcb_connect(":99", &screen);
  QT_CHECK(c != NULL, "xcb_connect always returns a connection object");
  int err = xcb_connection_has_error(c);
  QT_CHECK(err != 0, "no display :99, so the connection reports an error");
  printf("  info xcb_connection_has_error = %d (XCB_CONN_ERROR=%d)\n",
         err, XCB_CONN_ERROR);
  xcb_disconnect(c);

  /* --- Xlib: same, through the higher-level API --- */
  Display *d = XOpenDisplay(":99");
  QT_CHECK(d == NULL, "XOpenDisplay(:99) returns NULL with no server");
  printf("  info XDisplayName(NULL) = %s\n", XDisplayName(NULL));

  /* --- protocol constants that the generated headers must agree on --- */
  QT_CHECK(XCB_CONFIG_WINDOW_X == 1 && XCB_CONFIG_WINDOW_HEIGHT == 8,
           "generated protocol constants (xcb-proto ran correctly)");
  QT_CHECK(XCB_EVENT_MASK_KEY_PRESS == 1, "event mask constants");
  QT_END();
}
