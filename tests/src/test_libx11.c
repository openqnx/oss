/* libX11 without an X server: keysym name tables and case conversion, the
 * Xrm resource database (parse a resource file text, query with wildcards
 * and class names, merge, write out and read back), geometry parsing, the
 * region code, and XOpenDisplay failing cleanly on a display that is not
 * there. */
#include "common.h"
#include <stdlib.h>
#include <unistd.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xresource.h>
#include <X11/keysym.h>
static const char *s(const char *p) { return p ? p : "(null)"; }
static int rget(XrmDatabase db, const char *n, const char *c, const char *want) {
  char *type = NULL; XrmValue v = { 0, NULL };
  Bool ok = XrmGetResource(db, n, c, &type, &v);
  printf("  info %s -> %s\n", n, ok && v.addr ? (char *)v.addr : "(none)");
  return ok && v.addr && strcmp((char *)v.addr, want) == 0 && type && strcmp(type, "String") == 0;
}
int main(void) {
  QT_BEGIN("libx11");
  /* keysyms */
  QT_CHECK(XStringToKeysym("a") == XK_a && XStringToKeysym("Return") == XK_Return &&
           XStringToKeysym("F12") == XK_F12 && XStringToKeysym("udiaeresis") == XK_udiaeresis,
           "XStringToKeysym: a, Return, F12, udiaeresis");
  QT_CHECK(strcmp(s(XKeysymToString(XK_BackSpace)), "BackSpace") == 0 &&
           strcmp(s(XKeysymToString(XK_KP_Enter)), "KP_Enter") == 0, "XKeysymToString: BackSpace, KP_Enter");
  QT_CHECK(XStringToKeysym("NoSuchKeysymName") == NoSymbol, "unknown keysym name -> NoSymbol");
  KeySym lo, up;
  XConvertCase(XK_a, &lo, &up);
  int ok1 = lo == XK_a && up == XK_A;
  XConvertCase(XK_Odiaeresis, &lo, &up);
  int ok2 = lo == XK_odiaeresis && up == XK_Odiaeresis;
  XConvertCase(XK_Greek_sigma, &lo, &up);
  QT_CHECK(ok1 && ok2 && up == XK_Greek_SIGMA, "XConvertCase: Latin, Latin-1 and Greek");

  /* Xrm */
  XrmInitialize();
  const char *rc =
      "! comment line\n"
      "*background: gray20\n"
      "XTerm*foreground: white\n"
      "xterm.vt100.font: fixed\n"
      "app.button.label: Press \\\nme\n"
      "app*Button.width: 42\n";
  XrmDatabase db = XrmGetStringDatabase(rc);
  QT_CHECK(db != NULL, "XrmGetStringDatabase parses a resource text");
  QT_CHECK(rget(db, "xterm.vt100.font", "XTerm.VT100.Font", "fixed"), "exact resource name");
  QT_CHECK(rget(db, "xterm.vt100.foreground", "XTerm.VT100.Foreground", "white"), "class-based wildcard match");
  QT_CHECK(rget(db, "foo.bar.background", "Foo.Bar.Background", "gray20"), "global * wildcard");
  QT_CHECK(rget(db, "app.button.label", "App.Button.Label", "Press me"), "continued line joined");
  QT_CHECK(rget(db, "app.ok.width", "App.Button.Width", "42"), "class match on an inner component");
  char *t = NULL; XrmValue v;
  QT_CHECK(!XrmGetResource(db, "other.font", "Other.Font", &t, &v), "unmatched resource not found");
  XrmDatabase db2 = XrmGetStringDatabase("*background: black\nnew.thing: yes\n");
  XrmMergeDatabases(db2, &db);   /* db2 overrides, consumed */
  QT_CHECK(rget(db, "foo.background", "Foo.Background", "black") && rget(db, "new.thing", "New.Thing", "yes"),
           "XrmMergeDatabases overrides and adds");
  char path[64]; snprintf(path, sizeof path, "/tmp/qnxport-xrm-%d", (int)getpid());
  XrmPutFileDatabase(db, path);
  XrmDatabase db3 = XrmGetFileDatabase(path);
  QT_CHECK(db3 && rget(db3, "xterm.vt100.font", "XTerm.VT100.Font", "fixed") &&
           rget(db3, "foo.background", "Foo.Background", "black"), "XrmPutFileDatabase / XrmGetFileDatabase round trip");
  unlink(path);
  XrmDestroyDatabase(db3); XrmDestroyDatabase(db);
  QT_CHECK(XrmStringToQuark("qnx-quark") == XrmStringToQuark("qnx-quark") &&
           strcmp(s(XrmQuarkToString(XrmStringToQuark("qnx-quark"))), "qnx-quark") == 0, "quarks are interned");

  /* geometry */
  int x = 0, y = 0; unsigned w = 0, h = 0;
  int m = XParseGeometry("800x600+10-20", &x, &y, &w, &h);
  QT_CHECK((m & (WidthValue | HeightValue | XValue | YValue | YNegative)) == (WidthValue | HeightValue | XValue | YValue | YNegative) &&
           !(m & XNegative) && w == 800 && h == 600 && x == 10 && y == -20, "XParseGeometry 800x600+10-20");

  /* regions */
  Region r = XCreateRegion();
  XRectangle a = { 0, 0, 100, 50 }, b = { 50, 25, 100, 50 };
  XUnionRectWithRegion(&a, r, r);
  XUnionRectWithRegion(&b, r, r);
  XRectangle box;
  XClipBox(r, &box);
  QT_CHECK(box.x == 0 && box.y == 0 && box.width == 150 && box.height == 75, "region union bounding box 150x75");
  QT_CHECK(XPointInRegion(r, 120, 60) && XPointInRegion(r, 10, 10) && !XPointInRegion(r, 120, 10),
           "XPointInRegion inside/outside the L shape");
  Region q = XCreateRegion(), i = XCreateRegion();
  XUnionRectWithRegion(&b, q, q);
  XIntersectRegion(r, q, i);
  XClipBox(i, &box);
  QT_CHECK(box.x == 50 && box.width == 100 && box.height == 50, "XIntersectRegion");
  XDestroyRegion(r); XDestroyRegion(q); XDestroyRegion(i);

  /* no server */
  Display *d = XOpenDisplay(":97");
  QT_CHECK(d == NULL, "XOpenDisplay(:97) with no server returns NULL");
  if (d) XCloseDisplay(d);
  QT_END();
}
