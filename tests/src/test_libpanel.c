/* libpanel on a headless Wayland compositor (labwc, n10_labwc.h), with GTK 4
 * and libadwaita: panel_init; PanelPosition (area/column/row/depth)
 * serialised to a GVariant and back; a PanelSession with an item carrying
 * metadata saved to a GVariant and restored; a window with a PanelDock, a
 * PanelGrid receiving three PanelWidgets (titles, modified flag) and a
 * PanelFrame with two pages switched between, mapped on the compositor;
 * frames and pages counted, a widget raised and closed. */
#include "common.h"
#include "n10_labwc.h"
#include <libpanel.h>

static void spin(int ms) {
  for (int i = 0; i < ms / 10; i++) { while (g_main_context_iteration(NULL, FALSE)) ; usleep(10000); }
}
static void count_frame(PanelFrame *frame, gpointer ud) { (void)frame; (*(int *)ud)++; }
static PanelWidget *mkwidget(const char *title) {
  PanelWidget *w = PANEL_WIDGET(panel_widget_new());
  panel_widget_set_title(w, title);
  panel_widget_set_icon_name(w, "text-x-generic-symbolic");
  panel_widget_set_child(w, gtk_label_new(title));
  return w;
}

int main(void) {
  QT_BEGIN("libpanel");
  printf("  info libpanel %d.%d.%d\n", PANEL_MAJOR_VERSION, PANEL_MINOR_VERSION, PANEL_MICRO_VERSION);
  int up = n10_start_labwc(150);
  QT_CHECK(up, "labwc (headless) Wayland socket appeared");
  if (!up) { QT_END(); }
  QT_CHECK(gtk_init_check(), "gtk_init_check on labwc");
  adw_init();
  panel_init();
  QT_CHECK(g_type_is_a(PANEL_TYPE_WIDGET, GTK_TYPE_WIDGET) && g_type_is_a(PANEL_TYPE_DOCK, GTK_TYPE_WIDGET), "panel types registered");

  /* positions */
  PanelPosition *pos = panel_position_new();
  panel_position_set_area(pos, PANEL_AREA_START);
  panel_position_set_column(pos, 2);
  panel_position_set_row(pos, 1);
  panel_position_set_depth(pos, 3);
  GVariant *pv = g_variant_ref_sink(panel_position_to_variant(pos));
  char *ps = g_variant_print(pv, FALSE);
  printf("  info position: %s\n", ps); g_free(ps);
  PanelPosition *pos2 = panel_position_new_from_variant(pv);
  QT_CHECK(pos2 && panel_position_equal(pos, pos2) && panel_position_get_column(pos2) == 2 && panel_position_get_area(pos2) == PANEL_AREA_START,
           "PanelPosition round trip through a GVariant");
  QT_CHECK(!panel_position_is_indeterminate(pos) && panel_position_is_indeterminate(panel_position_new()), "a new position is indeterminate, a set one is not");

  /* session save and restore */
  PanelSession *ses = panel_session_new();
  PanelSessionItem *it = panel_session_item_new();
  panel_session_item_set_id(it, "doc-1");
  panel_session_item_set_module_name(it, "editor");
  panel_session_item_set_type_hint(it, "TextDocument");
  panel_session_item_set_position(it, pos);
  panel_session_item_set_metadata(it, "uri", "s", "file:///tmp/a.txt");
  panel_session_item_set_metadata(it, "line", "u", 42u);
  panel_session_append(ses, it);
  GVariant *sv = g_variant_ref_sink(panel_session_to_variant(ses));
  GError *err = NULL;
  PanelSession *ses2 = panel_session_new_from_variant(sv, &err);
  QT_CHECK(ses2 && panel_session_get_n_items(ses2) == 1, "PanelSession restored from its GVariant");
  if (err) { printf("  info %s\n", err->message); g_clear_error(&err); }
  if (ses2) {
    PanelSessionItem *back = panel_session_lookup_by_id(ses2, "doc-1");
    /* the format is a GVariant type string ("s", not the "&s" of
       g_variant_get), so the string comes back as a copy */
    char *uri = NULL; guint line = 0;
    QT_CHECK(back && !g_strcmp0(panel_session_item_get_module_name(back), "editor") &&
             panel_session_item_get_metadata(back, "uri", "s", &uri) && !g_strcmp0(uri, "file:///tmp/a.txt") &&
             panel_session_item_get_metadata(back, "line", "u", &line) && line == 42 &&
             panel_position_equal(panel_session_item_get_position(back), pos),
             "session item: module, metadata (uri, line) and position restored");
    g_free(uri);
  }

  /* widgets on screen */
  GtkWidget *win = adw_window_new();
  gtk_window_set_default_size(GTK_WINDOW(win), 800, 600);
  GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
  PanelDock *dock = PANEL_DOCK(panel_dock_new());
  PanelGrid *grid = PANEL_GRID(panel_grid_new());
  gtk_widget_set_vexpand(GTK_WIDGET(grid), TRUE);
  PanelWidget *w1 = mkwidget("Document 1"), *w2 = mkwidget("Document 2"), *w3 = mkwidget("Document 3");
  panel_widget_set_modified(w2, TRUE);
  panel_grid_add(grid, w1); panel_grid_add(grid, w2); panel_grid_add(grid, w3);
  PanelFrame *frame = PANEL_FRAME(panel_frame_new());
  PanelWidget *fa = mkwidget("Left A"), *fb = mkwidget("Left B");
  panel_frame_add(frame, fa); panel_frame_add(frame, fb);
  gtk_box_append(GTK_BOX(box), GTK_WIDGET(dock));
  gtk_box_append(GTK_BOX(box), GTK_WIDGET(grid));
  gtk_box_append(GTK_BOX(box), GTK_WIDGET(frame));
  adw_window_set_content(ADW_WINDOW(win), box);
  gtk_window_present(GTK_WINDOW(win));
  for (int i = 0; i < 100 && !gtk_widget_get_mapped(win); i++) spin(50);
  QT_CHECK(gtk_widget_get_mapped(win), "window with a dock, a grid and a frame mapped");
  spin(300);
  int frames = 0; panel_grid_foreach_frame(grid, count_frame, &frames);
  printf("  info grid: %u column(s), %d frame(s)\n", panel_grid_get_n_columns(grid), frames);
  QT_CHECK(frames >= 1 && panel_grid_get_n_columns(grid) >= 1, "grid holds the widgets in frames");
  QT_CHECK(!g_strcmp0(panel_widget_get_title(w2), "Document 2") && panel_widget_get_modified(w2) && !panel_widget_get_modified(w1),
           "widget title and modified flag");
  QT_CHECK(panel_frame_get_n_pages(frame) == 2, "frame with two pages");
  panel_frame_set_visible_child(frame, fa);
  spin(100);
  QT_CHECK(panel_frame_get_visible_child(frame) == fa, "frame switches to page A");
  panel_widget_raise(fb);
  spin(100);
  QT_CHECK(panel_frame_get_visible_child(frame) == fb, "panel_widget_raise brings page B forward");
  panel_widget_force_close(fb);
  spin(300);
  QT_CHECK(panel_frame_get_n_pages(frame) == 1, "a closed page leaves the frame");
  gtk_window_destroy(GTK_WINDOW(win));
  spin(100);
  g_variant_unref(pv); g_variant_unref(sv);
  n10_stop();
  QT_END();
}
