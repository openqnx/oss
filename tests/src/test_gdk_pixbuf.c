/* gdk-pixbuf: create a pixbuf, save it to PNG and JPEG in memory, and load it
 * back -- which proves the loader modules and their libpng/libjpeg links work. */
#include "common.h"
#include <gdk-pixbuf/gdk-pixbuf.h>
#define W 32
#define H 24
int main(void) {
  QT_BEGIN("gdk-pixbuf");
  printf("  info gdk-pixbuf %s\n", gdk_pixbuf_version);
  GdkPixbuf *pb = gdk_pixbuf_new(GDK_COLORSPACE_RGB, TRUE, 8, W, H);
  QT_CHECK(pb != NULL, "gdk_pixbuf_new");
  QT_CHECK(gdk_pixbuf_get_width(pb) == W && gdk_pixbuf_get_height(pb) == H,
           "pixbuf geometry");
  gdk_pixbuf_fill(pb, 0x3366CCFF);
  guchar *px = gdk_pixbuf_get_pixels(pb);
  QT_CHECK(px[0] == 0x33 && px[1] == 0x66 && px[2] == 0xCC && px[3] == 0xFF,
           "gdk_pixbuf_fill wrote RGBA");

  GSList *fmts = gdk_pixbuf_get_formats();
  int nfmt = g_slist_length(fmts);
  printf("  info %d loader format(s) available\n", nfmt);
  QT_CHECK(nfmt > 0, "at least one image format is registered");
  g_slist_free(fmts);

  const char *types[] = { "png", "jpeg" };
  for (size_t i = 0; i < sizeof types / sizeof types[0]; ++i) {
    gchar *buf = NULL; gsize n = 0; GError *err = NULL;
    gboolean ok = gdk_pixbuf_save_to_buffer(pb, &buf, &n, types[i], &err, NULL);
    QT_CHECK(ok && n > 0, types[i]);
    if (!ok) { if (err) { printf("  info %s\n", err->message); g_clear_error(&err); } continue; }
    GdkPixbufLoader *ld = gdk_pixbuf_loader_new();
    gdk_pixbuf_loader_write(ld, (const guchar *)buf, n, NULL);
    gdk_pixbuf_loader_close(ld, NULL);
    GdkPixbuf *back = gdk_pixbuf_loader_get_pixbuf(ld);
    QT_CHECK(back && gdk_pixbuf_get_width(back) == W &&
             gdk_pixbuf_get_height(back) == H, "decoded image has the right size");
    g_object_unref(ld);
    g_free(buf);
  }

  GdkPixbuf *scaled = gdk_pixbuf_scale_simple(pb, W / 2, H / 2, GDK_INTERP_BILINEAR);
  QT_CHECK(scaled && gdk_pixbuf_get_width(scaled) == W / 2, "gdk_pixbuf_scale_simple");
  if (scaled) g_object_unref(scaled);
  g_object_unref(pb);
  QT_END();
}
