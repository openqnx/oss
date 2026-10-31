/* tumbler (libtumbler + its plugins, offline, no D-Bus daemon): load the
 * thumbnailer providers from lib/tumbler-1/plugins, find the one for
 * image/png, generate a 300x200 PNG in /tmp and have it thumbnailed through
 * the XDG cache plugin; the thumbnail must appear at the freedesktop path
 * $XDG_CACHE_HOME/thumbnails/normal/<md5(uri)>.png, be 128 px wide with the
 * aspect ratio kept, carry Thumb::URI, and keep the source's colours. */
#include "common.h"
#include <stdlib.h>
#include <unistd.h>
#include <tumbler/tumbler.h>
#include <gdk-pixbuf/gdk-pixbuf.h>
static int ready, failed;
static void on_ready(TumblerThumbnailer *t, const gchar *uri, gpointer u) { (void)t; (void)u; printf("  info ready: %s\n", uri); ready++; }
static void on_error(TumblerThumbnailer *t, const gchar *uri, GQuark d, gint c, const gchar *m, gpointer u) {
  (void)t; (void)d; (void)u; printf("  info error for %s: %d %s\n", uri, c, m ? m : "(null)"); failed++;
}
int main(void) {
  QT_BEGIN("tumbler");
  char base[64]; snprintf(base, sizeof base, "/tmp/qnxport-tumbler-%d", (int)getpid());
  gchar *cache = g_build_filename(base, "cache", NULL), *cfg = g_build_filename(base, "config", NULL);
  g_mkdir_with_parents(cache, 0700); g_mkdir_with_parents(cfg, 0700);
  g_setenv("XDG_CACHE_HOME", cache, TRUE); g_setenv("XDG_CONFIG_HOME", cfg, TRUE);
  printf("  info tumbler %d.%d.%d\n", TUMBLER_MAJOR_VERSION, TUMBLER_MINOR_VERSION, TUMBLER_MICRO_VERSION);

  /* source image: left half red, right half blue */
  GdkPixbuf *src = gdk_pixbuf_new(GDK_COLORSPACE_RGB, FALSE, 8, 300, 200);
  guchar *p = gdk_pixbuf_get_pixels(src); int rs = gdk_pixbuf_get_rowstride(src);
  for (int y = 0; y < 200; y++) for (int x = 0; x < 300; x++) {
    guchar *q = p + y * rs + x * 3; q[0] = x < 150 ? 220 : 0; q[1] = 0; q[2] = x < 150 ? 0 : 220;
  }
  gchar *img = g_build_filename(base, "picture.png", NULL);
  QT_CHECK(gdk_pixbuf_save(src, img, "png", NULL, NULL), "source PNG written to /tmp");
  GFile *gf = g_file_new_for_path(img);
  gchar *uri = g_file_get_uri(gf);

  TumblerProviderFactory *fac = tumbler_provider_factory_get_default();
  GList *provs = tumbler_provider_factory_get_providers(fac, TUMBLER_TYPE_THUMBNAILER_PROVIDER);
  printf("  info %u thumbnailer provider(s)\n", g_list_length(provs));
  QT_CHECK(provs != NULL, "thumbnailer provider plugins loaded");
  TumblerThumbnailer *pick = NULL; int total = 0;
  for (GList *l = provs; l; l = l->next) {
    GList *ths = tumbler_thumbnailer_provider_get_thumbnailers(l->data);
    for (GList *t = ths; t; t = t->next) {
      total++;
      gchar **mt = tumbler_thumbnailer_get_mime_types(t->data);
      if (!pick && mt && g_strv_contains((const gchar *const *)mt, "image/png") &&
          tumbler_thumbnailer_supports_location(t->data, gf)) pick = g_object_ref(t->data);
    }
    g_list_free_full(ths, g_object_unref);
  }
  printf("  info %d thumbnailer(s) in total; for image/png: %s\n", total, pick ? G_OBJECT_TYPE_NAME(pick) : "(none)");
  QT_CHECK(pick != NULL, "a thumbnailer for image/png on file:// URIs");

  TumblerCache *tc = tumbler_cache_get_default();
  QT_CHECK(tc != NULL, "XDG cache plugin loaded");
  if (pick && tc) {
    TumblerThumbnailFlavor *fl = tumbler_cache_get_flavor(tc, "normal");
    QT_CHECK(fl != NULL, "cache has the 'normal' flavour");
    if (fl) {
      gint fw = 0, fh = 0; tumbler_thumbnail_flavor_get_size(fl, &fw, &fh);
      printf("  info flavour normal: %dx%d\n", fw, fh);
      TumblerFileInfo *info = tumbler_file_info_new(uri, "image/png", fl);
      GError *err = NULL;
      QT_CHECK(tumbler_file_info_load(info, NULL, &err), "tumbler_file_info_load");
      if (err) { printf("  info %s\n", err->message); g_clear_error(&err); }
      QT_CHECK(tumbler_file_info_needs_update(info), "no thumbnail yet: needs update");
      g_signal_connect(pick, "ready", G_CALLBACK(on_ready), NULL);
      g_signal_connect(pick, "error", G_CALLBACK(on_error), NULL);
      tumbler_thumbnailer_create(pick, NULL, info);
      for (int i = 0; i < 100 && !ready && !failed; i++) { while (g_main_context_iteration(NULL, FALSE)) ; if (!ready && !failed) usleep(50000); }
      QT_CHECK(ready == 1 && failed == 0, "thumbnailer reported ready");
      gchar *md5 = g_compute_checksum_for_string(G_CHECKSUM_MD5, uri, -1);
      gchar *name = g_strconcat(md5, ".png", NULL);
      gchar *tp = g_build_filename(cache, "thumbnails", "normal", name, NULL);
      printf("  info expecting %s\n", tp);
      QT_CHECK(g_file_test(tp, G_FILE_TEST_IS_REGULAR), "thumbnail at $XDG_CACHE_HOME/thumbnails/normal/<md5(uri)>.png");
      GdkPixbuf *th = gdk_pixbuf_new_from_file(tp, NULL);
      QT_CHECK(th != NULL, "thumbnail decodes");
      if (th) {
        int w = gdk_pixbuf_get_width(th), h = gdk_pixbuf_get_height(th);
        const gchar *tu = gdk_pixbuf_get_option(th, "tEXt::Thumb::URI");
        printf("  info thumbnail %dx%d, Thumb::URI %s\n", w, h, tu ? tu : "(none)");
        QT_CHECK(w == 128 && h >= 84 && h <= 86, "scaled to 128 wide, aspect kept");
        QT_CHECK(tu && strcmp(tu, uri) == 0, "Thumb::URI names the source");
        int n = gdk_pixbuf_get_n_channels(th), trs = gdk_pixbuf_get_rowstride(th);
        guchar *a = gdk_pixbuf_get_pixels(th) + (h / 2) * trs + 20 * n, *b = gdk_pixbuf_get_pixels(th) + (h / 2) * trs + 108 * n;
        printf("  info left %d,%d,%d right %d,%d,%d\n", a[0], a[1], a[2], b[0], b[1], b[2]);
        QT_CHECK(a[0] > 180 && a[2] < 40 && b[2] > 180 && b[0] < 40, "colours kept: red left, blue right");
        g_object_unref(th);
      }
      tumbler_file_info_load(info, NULL, NULL);
      QT_CHECK(!tumbler_file_info_needs_update(info), "after thumbnailing: up to date");
      unlink(tp); g_free(tp); g_free(name); g_free(md5);
      g_object_unref(info); g_object_unref(fl);
    }
  }
  if (tc) g_object_unref(tc);
  if (pick) g_object_unref(pick);
  unlink(img);
  QT_END();
}
