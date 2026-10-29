#include "common.h"
#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include <gdk-pixbuf/gdk-pixbuf.h>
#include <atk/atk.h>
/* GTK3 widgets may only be used after gtk_init() has opened a display. No
 * Wayland compositor runs on this target, so the widget layer is reported as
 * unexercised rather than driven into undefined behaviour. What does not need a
 * display is tested for real: the GObject type system GTK registers, ATK, and
 * gdk-pixbuf's encode/decode path. */
int main(int argc, char **argv) {
  QT_BEGIN("gtk3");
  printf("  info GTK %d.%d.%d, gdk-pixbuf %s, ATK %d.%d.%d\n",
         gtk_get_major_version(), gtk_get_minor_version(), gtk_get_micro_version(),
         gdk_pixbuf_version,
         atk_get_major_version(), atk_get_minor_version(), atk_get_micro_version());

  /* --- gdk-pixbuf: build an image, encode to PNG, decode it back --- */
  GdkPixbuf *pb = gdk_pixbuf_new(GDK_COLORSPACE_RGB, TRUE, 8, 32, 16);
  QT_CHECK(pb != NULL, "gdk_pixbuf_new");
  if (pb) {
    gdk_pixbuf_fill(pb, 0x3366ffff);
    QT_CHECK(gdk_pixbuf_get_width(pb) == 32 && gdk_pixbuf_get_height(pb) == 16,
             "pixbuf geometry");
    QT_CHECK(gdk_pixbuf_get_has_alpha(pb), "pixbuf has an alpha channel");
    gchar *buf = NULL; gsize len = 0; GError *err = NULL;
    gboolean ok = gdk_pixbuf_save_to_buffer(pb, &buf, &len, "png", &err, NULL);
    QT_CHECK(ok && len > 0, "gdk-pixbuf encoded a PNG in memory");
    if (ok) {
      GdkPixbufLoader *ld = gdk_pixbuf_loader_new();
      gdk_pixbuf_loader_write(ld, (const guchar *)buf, len, NULL);
      gdk_pixbuf_loader_close(ld, NULL);
      GdkPixbuf *back = gdk_pixbuf_loader_get_pixbuf(ld);
      QT_CHECK(back != NULL && gdk_pixbuf_get_width(back) == 32,
               "gdk-pixbuf decoded the PNG back");
      g_object_unref(ld); g_free(buf);
    } else if (err) { printf("  info png encode error: %s\n", err->message); g_error_free(err); }
    g_object_unref(pb);
  }

  /* --- GTK's types register without a display --- */
  /* GObject registers a type the first time its get_type() runs, so ask for it
   * that way rather than by name -- g_type_from_name() would just return 0 and
   * g_type_is_a(0, 0) is trivially true, which proves nothing. */
  GType widget_t = gtk_widget_get_type();
  GType button_t = gtk_button_get_type();
  QT_CHECK(widget_t != 0, "GtkWidget type registered");
  QT_CHECK(button_t != 0, "GtkButton type registered");
  QT_CHECK(g_type_is_a(button_t, widget_t), "GtkButton derives from GtkWidget");
  QT_CHECK(g_strcmp0(g_type_name(button_t), "GtkButton") == 0, "type name readback");
  QT_CHECK(gdk_display_get_type() != 0, "GdkDisplay type registered");

  /* --- ATK is usable standalone --- */
  AtkObject *ao = g_object_new(ATK_TYPE_OBJECT, NULL);
  QT_CHECK(ATK_IS_OBJECT(ao), "AtkObject instantiated");
  atk_object_set_name(ao, "brake-pedal");
  QT_CHECK(g_strcmp0(atk_object_get_name(ao), "brake-pedal") == 0, "ATK name round trip");
  g_object_unref(ao);

  /* --- the display layer: failing to connect is the expected outcome here --- */
  gboolean opened = gtk_init_check(&argc, &argv);
  printf("  info gtk_init_check -> %s\n", opened ? "display opened" : "no display");
  if (opened) {
    /* With a display open the widget layer is fully usable. */
    GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 6);
    GtkWidget *label = gtk_label_new("Wavefront");
    GtkWidget *btn = gtk_button_new_with_label("OK");
    QT_CHECK(GTK_IS_BOX(box) && GTK_IS_LABEL(label) && GTK_IS_BUTTON(btn),
             "widgets constructed with a live display");
    QT_CHECK(g_strcmp0(gtk_label_get_text(GTK_LABEL(label)), "Wavefront") == 0,
             "label text round trip");
    gtk_container_add(GTK_CONTAINER(box), label);
    gtk_container_add(GTK_CONTAINER(box), btn);
    GList *kids = gtk_container_get_children(GTK_CONTAINER(box));
    QT_CHECK(g_list_length(kids) == 2, "container holds both children");
    g_list_free(kids);

    AtkObject *acc = gtk_widget_get_accessible(btn);
    QT_CHECK(ATK_IS_OBJECT(acc), "gtk_widget_get_accessible");
    QT_CHECK(atk_object_get_role(acc) == ATK_ROLE_PUSH_BUTTON,
             "button reports the ATK push-button role");

    GtkWidget *win = gtk_offscreen_window_new();
    gtk_container_add(GTK_CONTAINER(win), box);
    gtk_widget_show_all(win);
    GtkRequisition req;
    gtk_widget_get_preferred_size(box, NULL, &req);
    QT_CHECK(req.width > 0 && req.height > 0, "widget tree reports a natural size");
    printf("  info natural size %dx%d px\n", req.width, req.height);

    /* Render the live widget tree through cairo and write it out. */
    cairo_surface_t *s = cairo_image_surface_create(CAIRO_FORMAT_ARGB32,
                                                    req.width > 0 ? req.width : 120,
                                                    req.height > 0 ? req.height : 60);
    cairo_t *cr = cairo_create(s);
    gtk_widget_draw(box, cr);
    QT_CHECK(cairo_status(cr) == CAIRO_STATUS_SUCCESS, "gtk_widget_draw onto cairo");
    QT_CHECK(cairo_surface_write_to_png(s, "/tmp/gtk3-widgets.png") == CAIRO_STATUS_SUCCESS,
             "rendered widget tree written as PNG");
    cairo_destroy(cr); cairo_surface_destroy(s);
    gtk_widget_destroy(win);
  } else {
    printf("  info widget layer not exercised: no display. Start broadwayd and set\n"
           "       GDK_BACKEND=broadway BROADWAY_DISPLAY=:5 to cover it.\n");
    QT_CHECK(gdk_display_get_default() == NULL, "no default display, as expected");
  }
  QT_END();
}
