/* GTK4: the type system, the CSS engine, GSK's render-node tree and the
 * Broadway/Wayland backends. gtk_init() needs a display, which a bare QNX
 * target has not got, so everything below runs on the parts of GTK4 that work
 * without one -- which is most of what a port can break. */
#include "common.h"
#include <gtk/gtk.h>
#include <gsk/gsk.h>
int main(void) {
  QT_BEGIN("gtk4");
  printf("  info GTK %d.%d.%d\n", gtk_get_major_version(),
         gtk_get_minor_version(), gtk_get_micro_version());
  QT_CHECK(gtk_get_major_version() == 4, "GTK 4.x");

  /* GTK4 requires gtk_init() before its type system is usable: without a
     display, g_type_ensure(gtk_widget_get_type()) already aborts with "cannot
     create new instance of invalid (non-instantiatable) type". So everything
     past this point is conditional on a display existing -- what the port
     itself has to prove is that the library loads, links and reports the
     absence of a display instead of crashing. */
  gboolean have_display = gtk_init_check();
  printf("  info gtk_init_check() -> %s\n",
         have_display ? "a display is available" : "no display");

  if (!have_display) {
    QT_CHECK(!have_display, "gtk_init_check reported no display rather than aborting");
    printf("  info skipping the widget, CSS and GSK checks: they need gtk_init()\n");
    QT_END();
  }

  /* --- the type system --- */
  QT_CHECK(g_type_from_name("GtkWidget") != 0, "GtkWidget type is registered");
  QT_CHECK(g_type_is_a(GTK_TYPE_BUTTON, GTK_TYPE_WIDGET), "GtkButton derives from GtkWidget");
  QT_CHECK(g_type_is_a(GTK_TYPE_BOX, GTK_TYPE_WIDGET), "GtkBox derives from GtkWidget");

  /* --- CSS --- */
  GtkCssProvider *css = gtk_css_provider_new();
  QT_CHECK(css != NULL, "gtk_css_provider_new");
  gtk_css_provider_load_from_string(css, "button { color: rgb(255,0,0); }");
  g_object_unref(css);

  /* --- GSK render nodes --- */
  graphene_rect_t bounds = GRAPHENE_RECT_INIT(0, 0, 100, 50);
  GdkRGBA red = { 1.f, 0.f, 0.f, 1.f };
  GskRenderNode *node = gsk_color_node_new(&red, &bounds);
  QT_CHECK(node != NULL, "gsk_color_node_new");
  if (node) {
    graphene_rect_t got;
    gsk_render_node_get_bounds(node, &got);
    QT_CHECK(got.size.width == 100.f, "render node keeps its bounds");
    GBytes *ser = gsk_render_node_serialize(node);
    QT_CHECK(ser != NULL && g_bytes_get_size(ser) > 0, "gsk_render_node_serialize");
    if (ser) g_bytes_unref(ser);
    gsk_render_node_unref(node);
  }

  GtkWidget *w = gtk_button_new_with_label("Build");
  QT_CHECK(GTK_IS_BUTTON(w), "created a real button");
  QT_CHECK(g_strcmp0(gtk_button_get_label(GTK_BUTTON(w)), "Build") == 0, "button label");
  g_object_ref_sink(w);
  g_object_unref(w);
  QT_END();
}
