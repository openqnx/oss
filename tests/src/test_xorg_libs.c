#include "common.h"
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <libxcvt/libxcvt.h>
#include <X11/xshmfence.h>
#include <xcb/xcb_event.h>
#include <X11/Xlib.h>
#include <X11/Intrinsic.h>
#include <X11/Xmu/CharSet.h>
#include <X11/ICE/ICElib.h>
#include <X11/SM/SMlib.h>
#include <X11/XKBlib.h>
#include <X11/extensions/XKBrules.h>
#include <X11/fonts/fontstruct.h>
#include <X11/fonts/libxfont2.h>
/* The X.Org libraries that need no X server, each with a known answer:
 * libxcvt (the CVT modeline of 1920x1080@60), libxshmfence (a fence in
 * shared memory: trigger, query, reset), xcb-util (event names), libXmu
 * (ISO Latin-1 case folding), libICE (listening for connections), libSM
 * (no session manager: the documented error), libXt (an application
 * context), libxkbfile (XKB rules -> keymap components), libXfont2 (the
 * font pattern cache). */
int main(void) {
  QT_BEGIN("xorg-libs");

  struct libxcvt_mode_info *m = libxcvt_gen_mode_info(1920, 1080, 60.0f, false, false);
  QT_CHECK(m != NULL, "libxcvt_gen_mode_info");
  if (m) {
    printf("  info cvt 1920x1080@60: %.2f MHz %u %u %u %u  %u %u %u %u\n", m->dot_clock / 1000.0,
           m->hdisplay, m->hsync_start, m->hsync_end, m->htotal, m->vdisplay, m->vsync_start, m->vsync_end, m->vtotal);
    QT_CHECK(m->dot_clock / 1000 == 173 && m->htotal == 2576 && m->hsync_start == 2048 && m->hsync_end == 2248,
             "horizontal timing = cvt's (173.00 MHz, 2048 2248 2576)");
    QT_CHECK(m->vsync_start == 1083 && m->vsync_end == 1088 && m->vtotal == 1120, "vertical timing = cvt's (1083 1088 1120)");
    free(m);
  }

  int fd = xshmfence_alloc_shm();
  QT_CHECK(fd >= 0, "xshmfence_alloc_shm");
  if (fd >= 0) {
    struct xshmfence *f = xshmfence_map_shm(fd);
    QT_CHECK(f != NULL, "xshmfence_map_shm");
    if (f) {
      QT_CHECK(xshmfence_query(f) == 0, "new fence not triggered");
      xshmfence_trigger(f);
      QT_CHECK(xshmfence_query(f) == 1, "triggered");
      QT_CHECK(xshmfence_await(f) == 0, "await on a triggered fence returns");
      xshmfence_reset(f);
      QT_CHECK(xshmfence_query(f) == 0, "reset");
      xshmfence_unmap_shm(f);
    }
    close(fd);
  }

  const char *l = xcb_event_get_label(XCB_KEY_PRESS);
  QT_CHECK(l && strcmp(l, "KeyPress") == 0, "xcb_event_get_label(KEY_PRESS) = KeyPress");

  char low[16]; XmuCopyISOLatin1Lowered(low, "HeLLo\xC9");
  QT_CHECK(strcmp(low, "hello\xE9") == 0, "XmuCopyISOLatin1Lowered (incl. E-acute)");
  QT_CHECK(XmuCompareISOLatin1("Foo", "fOO") == 0, "XmuCompareISOLatin1 ignores case");

  int n = 0; IceListenObj *lo = NULL; char err[256] = "";
  Status st = IceListenForConnections(&n, &lo, sizeof err, err);
  QT_CHECK(st && n > 0, "IceListenForConnections");
  if (st && n > 0) {
    char *ids = IceComposeNetworkIdList(n, lo);
    printf("  info ICE listening: %s\n", ids ? ids : "?");
    QT_CHECK(ids && (strstr(ids, "local/") || strstr(ids, "tcp/")), "network id list");
    free(ids); IceFreeListenObjs(n, lo);
  } else printf("  info %s\n", err);

  unsetenv("SESSION_MANAGER");
  char *cid = NULL; SmcCallbacks cb; memset(&cb, 0, sizeof cb);
  SmcConn sc = SmcOpenConnection(NULL, NULL, SmProtoMajor, SmProtoMinor, 0, &cb, NULL, &cid, sizeof err, err);
  printf("  info SmcOpenConnection: %s\n", err);
  QT_CHECK(sc == NULL && strstr(err, "SESSION_MANAGER") != NULL, "no session manager: the documented error");

  XtToolkitInitialize();
  XtAppContext app = XtCreateApplicationContext();
  QT_CHECK(app != NULL, "XtCreateApplicationContext");
  String s = XtNewString("qnx");
  QT_CHECK(s && strcmp(s, "qnx") == 0, "XtNewString");
  XtFree(s);
  if (app) XtDestroyApplicationContext(app);

  char rf[] = "/tmp/xkbrules-XXXXXX"; int rfd = mkstemp(rf);
  FILE *r = fdopen(rfd, "w");
  fputs("! model = keycodes\n  *  = evdev\n\n! layout = symbols\n  *  = pc+%l\n\n"
        "! model = types\n  *  = complete\n\n! model = compat\n  *  = complete\n\n! model = geometry\n  *  = pc(%m)\n", r);
  fclose(r);
  XkbRF_RulesPtr rules = XkbRF_Create(0, 0);
  r = fopen(rf, "r");
  QT_CHECK(rules && XkbRF_LoadRules(r, rules), "XkbRF_LoadRules");
  fclose(r); unlink(rf);
  if (rules) {
    XkbRF_VarDefsRec v; memset(&v, 0, sizeof v); v.model = "pc105"; v.layout = "us";
    XkbComponentNamesRec names; memset(&names, 0, sizeof names);
    QT_CHECK(XkbRF_GetComponents(rules, &v, &names), "XkbRF_GetComponents");
    printf("  info keycodes=%s symbols=%s types=%s\n", names.keycodes, names.symbols, names.types);
    QT_CHECK(names.keycodes && strcmp(names.keycodes, "evdev") == 0, "keycodes = evdev");
    QT_CHECK(names.symbols && strcmp(names.symbols, "pc+us") == 0, "symbols = pc+us");
    QT_CHECK(names.geometry && strcmp(names.geometry, "pc(pc105)") == 0, "geometry = pc(pc105)");
    XkbRF_Free(rules, True);
  }

  xfont2_pattern_cache_ptr pc = xfont2_make_font_pattern_cache();
  QT_CHECK(pc != NULL, "xfont2_make_font_pattern_cache");
  if (pc) {
    FontPtr fake = (FontPtr)(void *)&pc;
    const char *pat = "-misc-fixed-medium-r-normal--13-*";
    xfont2_cache_font_pattern(pc, pat, (int)strlen(pat), fake);
    QT_CHECK(xfont2_find_cached_font_pattern(pc, pat, (int)strlen(pat)) == fake, "cached pattern found");
    QT_CHECK(xfont2_find_cached_font_pattern(pc, "nope", 4) == NULL, "other pattern not found");
    xfont2_free_font_pattern_cache(pc);
  }
  QT_END();
}
