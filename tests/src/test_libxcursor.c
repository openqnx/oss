/* libXcursor without a server: build a two-size cursor with a comment, save
 * it in the Xcursor file format, check the file magic, load it back (all
 * images, and best-size selection), then install it as a theme in /tmp
 * with an inheriting second theme and resolve it via XCURSOR_PATH the way
 * a toolkit looks up "left_ptr". */
#include "common.h"
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <X11/Xlib.h>
#include <X11/cursorfont.h>
#include <X11/Xcursor/Xcursor.h>
static XcursorImage *mk(int sz, XcursorPixel argb) {
  XcursorImage *im = XcursorImageCreate(sz, sz);
  if (!im) return NULL;
  im->size = sz; im->xhot = sz / 4; im->yhot = sz / 3; im->delay = 50;
  for (int i = 0; i < sz * sz; i++) im->pixels[i] = (i % (sz + 1)) ? argb : 0xff000000u;   /* black diagonal */
  return im;
}
static int same(const XcursorImage *a, const XcursorImage *b) {
  return a && b && a->width == b->width && a->height == b->height && a->xhot == b->xhot && a->yhot == b->yhot &&
         a->delay == b->delay && memcmp(a->pixels, b->pixels, a->width * a->height * 4) == 0;
}
static void mkdirs(const char *p) { char b[256]; snprintf(b, sizeof b, "%s", p); for (char *s = b + 1; *s; s++) if (*s == '/') { *s = 0; mkdir(b, 0755); *s = '/'; } mkdir(b, 0755); }
int main(void) {
  QT_BEGIN("libxcursor");
  XcursorImages *imgs = XcursorImagesCreate(2);
  XcursorImage *a = mk(24, 0xffff8000u), *b = mk(32, 0x80204060u);
  QT_CHECK(imgs && a && b, "XcursorImagesCreate / XcursorImageCreate");
  if (!imgs || !a || !b) QT_END();
  imgs->images[0] = a; imgs->images[1] = b; imgs->nimage = 2;
  XcursorComments *cm = XcursorCommentsCreate(1);
  XcursorComment *c = XcursorCommentCreate(XCURSOR_COMMENT_COPYRIGHT, 16);
  strcpy(c->comment, "QNX port test");
  cm->comments[0] = c; cm->ncomment = 1;

  char base[64], file[128]; snprintf(base, sizeof base, "/tmp/qnxport-xcursor-%d", (int)getpid());
  mkdirs(base);
  snprintf(file, sizeof file, "%s/arrow.xcur", base);
  FILE *f = fopen(file, "wb");
  QT_CHECK(f && XcursorFileSave(f, cm, imgs), "XcursorFileSave (2 images + comment)");
  if (f) fclose(f);
  unsigned char mg[4] = { 0 };
  f = fopen(file, "rb"); if (f) { (void)!fread(mg, 1, 4, f); fclose(f); }
  QT_CHECK(memcmp(mg, "Xcur", 4) == 0, "file starts with the Xcur magic");

  XcursorComments *rc = NULL; XcursorImages *ri = NULL;
  f = fopen(file, "rb");
  QT_CHECK(f && XcursorFileLoad(f, &rc, &ri), "XcursorFileLoad");
  if (f) fclose(f);
  QT_CHECK(ri && ri->nimage == 2 && same(ri->images[0], a) && same(ri->images[1], b), "both images read back identical");
  QT_CHECK(rc && rc->ncomment == 1 && rc->comments[0]->comment_type == XCURSOR_COMMENT_COPYRIGHT &&
           strcmp(rc->comments[0]->comment, "QNX port test") == 0, "comment read back");
  if (ri) XcursorImagesDestroy(ri);
  if (rc) XcursorCommentsDestroy(rc);
  XcursorImage *best = XcursorFilenameLoadImage(file, 30);
  printf("  info best match for size 30: %d\n", best ? (int)best->size : -1);
  QT_CHECK(best && best->size == 32 && same(best, b), "nearest size chosen (30 -> 32)");
  if (best) XcursorImageDestroy(best);

  /* theme lookup: child theme inherits from parent, which has left_ptr */
  char d[192], p[256];
  snprintf(d, sizeof d, "%s/icons/qnxparent/cursors", base); mkdirs(d);
  snprintf(p, sizeof p, "%s/left_ptr", d);
  QT_CHECK(XcursorFilenameSaveImages(p, imgs), "XcursorFilenameSaveImages into a theme");
  snprintf(d, sizeof d, "%s/icons/qnxchild", base); mkdirs(d);
  snprintf(p, sizeof p, "%s/index.theme", d);
  f = fopen(p, "w"); if (f) { fputs("[Icon Theme]\nName=qnxchild\nInherits=qnxparent\n", f); fclose(f); }
  snprintf(p, sizeof p, "%s/icons", base);
  setenv("XCURSOR_PATH", p, 1);
  const char *lp = XcursorLibraryPath();
  printf("  info XcursorLibraryPath = %s\n", lp ? lp : "(null)");
  QT_CHECK(lp && strcmp(lp, p) == 0, "XcursorLibraryPath honours XCURSOR_PATH");
  XcursorImages *th = XcursorLibraryLoadImages("left_ptr", "qnxchild", 24);
  QT_CHECK(th && th->nimage >= 1 && same(th->images[0], a), "left_ptr resolved through Inherits= at size 24");
  if (th) XcursorImagesDestroy(th);
  QT_CHECK(XcursorLibraryLoadImages("no_such_cursor", "qnxchild", 24) == NULL, "missing cursor name -> NULL");
  QT_CHECK(XcursorLibraryShape("left_ptr") == XC_left_ptr && XcursorLibraryShape("watch") == XC_watch,
           "XcursorLibraryShape maps names to cursorfont shapes");

  XcursorImagesDestroy(imgs);
  XcursorCommentsDestroy(cm);
  /* clean up */
  snprintf(p, sizeof p, "%s/icons/qnxparent/cursors/left_ptr", base); unlink(p);
  snprintf(p, sizeof p, "%s/icons/qnxchild/index.theme", base); unlink(p);
  snprintf(p, sizeof p, "%s/icons/qnxparent/cursors", base); rmdir(p);
  snprintf(p, sizeof p, "%s/icons/qnxparent", base); rmdir(p);
  snprintf(p, sizeof p, "%s/icons/qnxchild", base); rmdir(p);
  snprintf(p, sizeof p, "%s/icons", base); rmdir(p);
  unlink(file); rmdir(base);
  QT_END();
}
