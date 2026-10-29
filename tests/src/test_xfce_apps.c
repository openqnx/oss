/* The Xfce application layer, and the X11 path that Thunar forced into GTK3.
 *
 *  - GTK3 now carries its X11 backend and cairo its Xlib surface.  The check is
 *    that the backend really is compiled in and its types register -- a build
 *    that silently dropped it would still link.
 *  - libxfce4panel: the ABI every panel plugin is written against.
 *  - thunarx: the extension library Thunar's plugins load through; its
 *    provider factory scans a plugin directory, which has to work on target. */
#include "common.h"
#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include <cairo.h>
#include <cairo-xlib.h>
#ifdef GDK_WINDOWING_X11
#include <gdk/gdkx.h>
#endif
#include <libxfce4panel/libxfce4panel.h>
#include <thunarx/thunarx.h>

int main(void) {
  QT_BEGIN("xfce apps / gtk3-x11 / cairo-xlib");

#ifdef GDK_WINDOWING_X11
  QT_CHECK(1, "GTK3 was built with GDK_WINDOWING_X11");
  GType xd = gdk_x11_display_get_type();
  QT_CHECK(xd != 0 && g_type_is_a(xd, GDK_TYPE_DISPLAY),
           "GdkX11Display registered and is a GdkDisplay");
#else
  QT_CHECK(0, "GTK3 was built with GDK_WINDOWING_X11");
#endif
#ifdef GDK_WINDOWING_WAYLAND
  QT_CHECK(1, "Wayland backend still present alongside X11");
#else
  QT_CHECK(0, "Wayland backend still present alongside X11");
#endif

#if CAIRO_HAS_XLIB_SURFACE
  QT_CHECK(1, "cairo was built with the Xlib surface");
#else
  QT_CHECK(0, "cairo was built with the Xlib surface");
#endif
  /* The Xlib surface needs a Display to create one.  Taking these addresses
     puts them in the GOT, which is bound when the binary loads -- so if
     libcairo lacked them this test would not start.  The comparison itself is
     trivially true; the load is the check. */
  QT_CHECK(cairo_xlib_surface_create != NULL && cairo_xlib_surface_get_width != NULL,
           "cairo_xlib_* resolved from libcairo");

  /* Without a display, image surfaces still exercise pixman through cairo. */
  cairo_surface_t *img = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, 8, 8);
  cairo_t *cr = cairo_create(img);
  cairo_set_source_rgb(cr, 1, 0, 0);
  cairo_paint(cr);
  cairo_surface_flush(img);
  const unsigned char *px = cairo_image_surface_get_data(img);
  QT_CHECK(px && px[2] == 0xff && px[1] == 0 && px[0] == 0, "cairo painted red (BGRA)");
  cairo_destroy(cr); cairo_surface_destroy(img);

  /* --- libxfce4panel --- */
  printf("  info libxfce4panel %u.%u.%u\n", libxfce4panel_major_version,
         libxfce4panel_minor_version, libxfce4panel_micro_version);
  QT_CHECK(libxfce4panel_check_version(4, 20, 0) == NULL,
           "libxfce4panel_check_version(4.20.0)");
  GType plugin_t = xfce_panel_plugin_get_type();
  QT_CHECK(plugin_t != 0 && g_type_is_a(plugin_t, GTK_TYPE_EVENT_BOX),
           "XfcePanelPlugin registered as a GtkEventBox");
  QT_CHECK(xfce_panel_plugin_provider_get_type() != 0,
           "XfcePanelPluginProvider interface registered");

  /* --- thunarx --- */
  printf("  info thunarx %u.%u.%u\n", thunarx_major_version,
         thunarx_minor_version, thunarx_micro_version);
  QT_CHECK(thunarx_check_version(4, 20, 0) == NULL, "thunarx_check_version(4.20.0)");
  ThunarxProviderFactory *f = thunarx_provider_factory_get_default();
  QT_CHECK(f != NULL, "thunarx_provider_factory_get_default");
  if (f) {
    /* Scans $libdir/thunarx-3 for extension modules; none are installed in the
       bundle, so an empty list is the correct answer -- the point is that the
       directory scan and GTypeModule machinery run without error. */
    GList *p = thunarx_provider_factory_list_providers(f, THUNARX_TYPE_MENU_PROVIDER);
    printf("  info %u menu provider(s)\n", g_list_length(p));
    g_list_free_full(p, g_object_unref);
    g_object_unref(f);
  }

  QT_END();
}
