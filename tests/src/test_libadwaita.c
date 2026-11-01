/* libadwaita on a headless Wayland compositor (labwc, n10_labwc.h): adw_init,
 * the Adwaita style sheet compiled into the library (it is generated from
 * SCSS at build time), the style manager switching to dark, an
 * AdwApplicationWindow with a header bar, view stack, tab view, toast
 * overlay and a preferences group mapped on the compositor, breakpoint
 * conditions parsed and printed, adw_lerp and length units, spring
 * parameters, and an AdwAboutDialog filled in from an AppStream metainfo
 * file (through libappstream). */
#include "common.h"
#include "n10_labwc.h"
#include <adwaita.h>

static void spin(int ms) {
  for (int i = 0; i < ms / 10; i++) { while (g_main_context_iteration(NULL, FALSE)) ; usleep(10000); }
}
static const char METAINFO[] =
  "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
  "<component type=\"desktop-application\">\n"
  "  <id>org.qnx.PortTest</id>\n"
  "  <name>QNX Port Test</name>\n"
  "  <summary>Checks libadwaita on QNX</summary>\n"
  "  <metadata_license>CC0-1.0</metadata_license>\n"
  "  <project_license>Apache-2.0</project_license>\n"
  "  <developer id=\"org.qnx\"><name>QNX Porters</name></developer>\n"
  "  <url type=\"homepage\">https://example.org/qnx</url>\n"
  "  <url type=\"bugtracker\">https://example.org/qnx/issues</url>\n"
  "  <releases>\n"
  "    <release version=\"2.1\" date=\"2026-10-01\"><description><p>Second.</p></description></release>\n"
  "    <release version=\"1.0\" date=\"2025-01-01\"><description><p>First.</p></description></release>\n"
  "  </releases>\n"
  "</component>\n";

int main(void) {
  QT_BEGIN("libadwaita");
  printf("  info libadwaita %d.%d.%d\n", adw_get_major_version(), adw_get_minor_version(), adw_get_micro_version());
  QT_CHECK(adw_get_major_version() == 1 && adw_get_minor_version() == 6, "libadwaita 1.6");

  /* metainfo for the about dialog, served from a resource overlay */
  char dir[96], path[160], overlay[200];
  snprintf(dir, sizeof dir, "/tmp/qnxport-adw-%d", (int)getpid());
  mkdir(dir, 0755);
  snprintf(path, sizeof path, "%s/metainfo.xml", dir);
  FILE *f = fopen(path, "w"); fputs(METAINFO, f); fclose(f);
  snprintf(overlay, sizeof overlay, "/org/qnx/PortTest=%s", dir);
  setenv("G_RESOURCE_OVERLAYS", overlay, 1);

  /* breakpoints, lerp and units need no display */
  AdwBreakpointCondition *c = adw_breakpoint_condition_parse("max-width: 400sp and min-aspect-ratio: 4/3");
  char *cs = c ? adw_breakpoint_condition_to_string(c) : NULL;
  printf("  info condition: %s\n", cs ? cs : "(null)");
  QT_CHECK(cs && !strcmp(cs, "max-width: 400sp and min-aspect-ratio: 4/3"), "breakpoint condition parsed and printed back");
  g_free(cs); if (c) adw_breakpoint_condition_free(c);
  QT_CHECK(adw_lerp(10.0, 20.0, 0.25) == 12.5, "adw_lerp(10, 20, 0.25) = 12.5");
  AdwSpringParams *sp = adw_spring_params_new(1.0, 1.0, 100.0);
  QT_CHECK(sp && adw_spring_params_get_damping(sp) == 20.0, "spring params: damping ratio 1, mass 1, stiffness 100 -> damping 20");
  if (sp) adw_spring_params_unref(sp);

  int up = n10_start_labwc(150);
  QT_CHECK(up, "labwc (headless) Wayland socket appeared");
  if (!up) { QT_END(); }
  QT_CHECK(gtk_init_check(), "gtk_init_check on labwc");
  adw_init();
  QT_CHECK(adw_is_initialized(), "adw_init");
  QT_CHECK(adw_length_unit_to_px(ADW_LENGTH_UNIT_PT, 72.0, NULL) == 96.0, "72 pt = 96 px");

  GBytes *css = g_resources_lookup_data("/org/gnome/Adwaita/styles/base.css", 0, NULL);
  printf("  info base.css: %zu bytes\n", css ? g_bytes_get_size(css) : (size_t)0);
  QT_CHECK(css && g_bytes_get_size(css) > 10000 && strstr(g_bytes_get_data(css, NULL), ".navigation-sidebar"),
           "Adwaita style sheet (compiled from SCSS) is in the library");
  if (css) g_bytes_unref(css);

  AdwStyleManager *sm = adw_style_manager_get_default();
  adw_style_manager_set_color_scheme(sm, ADW_COLOR_SCHEME_FORCE_DARK);
  QT_CHECK(adw_style_manager_get_dark(sm), "style manager: FORCE_DARK makes it dark");
  adw_style_manager_set_color_scheme(sm, ADW_COLOR_SCHEME_FORCE_LIGHT);
  QT_CHECK(!adw_style_manager_get_dark(sm), "style manager: FORCE_LIGHT makes it light");

  GtkWidget *win = adw_application_window_new(NULL);
  gtk_window_set_default_size(GTK_WINDOW(win), 640, 480);
  GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
  gtk_box_append(GTK_BOX(box), adw_header_bar_new());
  AdwViewStack *vs = ADW_VIEW_STACK(adw_view_stack_new());
  adw_view_stack_add_titled(vs, gtk_label_new("one"), "one", "One");
  AdwViewStackPage *p2 = adw_view_stack_add_titled(vs, gtk_label_new("two"), "two", "Two");
  adw_view_stack_set_visible_child_name(vs, "two");
  AdwTabView *tv = ADW_TAB_VIEW(adw_tab_view_new());
  for (int i = 0; i < 3; i++) { AdwTabPage *tp = adw_tab_view_append(tv, gtk_label_new("tab")); adw_tab_page_set_title(tp, i == 2 ? "third" : "tab"); }
  AdwToastOverlay *to = ADW_TOAST_OVERLAY(adw_toast_overlay_new());
  adw_toast_overlay_set_child(to, GTK_WIDGET(vs));
  AdwPreferencesGroup *pg = ADW_PREFERENCES_GROUP(adw_preferences_group_new());
  adw_preferences_group_set_title(pg, "Settings");
  GtkWidget *row = adw_action_row_new();
  adw_preferences_row_set_title(ADW_PREFERENCES_ROW(row), "Row");
  adw_action_row_set_subtitle(ADW_ACTION_ROW(row), "subtitle");
  adw_preferences_group_add(pg, row);
  gtk_box_append(GTK_BOX(box), GTK_WIDGET(to));
  gtk_box_append(GTK_BOX(box), GTK_WIDGET(tv));
  gtk_box_append(GTK_BOX(box), GTK_WIDGET(pg));
  adw_application_window_set_content(ADW_APPLICATION_WINDOW(win), box);
  gtk_window_present(GTK_WINDOW(win));
  for (int i = 0; i < 100 && !gtk_widget_get_mapped(win); i++) spin(50);
  QT_CHECK(gtk_widget_get_mapped(win), "AdwApplicationWindow mapped on the compositor");
  spin(300);
  QT_CHECK(!strcmp(adw_view_stack_get_visible_child_name(vs), "two") && adw_view_stack_get_page(vs, adw_view_stack_get_visible_child(vs)) == p2,
           "view stack shows page two");
  QT_CHECK(adw_tab_view_get_n_pages(tv) == 3 && !strcmp(adw_tab_page_get_title(adw_tab_view_get_nth_page(tv, 2)), "third"), "tab view with three pages");
  adw_tab_view_close_page(tv, adw_tab_view_get_nth_page(tv, 0));
  spin(200);
  QT_CHECK(adw_tab_view_get_n_pages(tv) == 2, "closing a tab leaves two");
  AdwToast *toast = adw_toast_new("Saved");
  adw_toast_overlay_add_toast(to, toast);
  spin(100);
  QT_CHECK(!strcmp(adw_toast_get_title(toast), "Saved"), "toast shown");
  QT_CHECK(gtk_widget_get_width(win) > 0 && gtk_widget_get_height(win) > 0, "window was allocated a size");
  printf("  info window %dx%d\n", gtk_widget_get_width(win), gtk_widget_get_height(win));

  AdwDialog *ad = adw_about_dialog_new_from_appdata("/org/qnx/PortTest/metainfo.xml", "2.1");
  QT_CHECK(ad != NULL, "AdwAboutDialog from AppStream metainfo");
  if (ad) {
    AdwAboutDialog *a = ADW_ABOUT_DIALOG(ad);
    printf("  info about: %s %s by %s, %s\n", adw_about_dialog_get_application_name(a), adw_about_dialog_get_version(a),
           adw_about_dialog_get_developer_name(a), adw_about_dialog_get_website(a));
    QT_CHECK(!g_strcmp0(adw_about_dialog_get_application_name(a), "QNX Port Test") &&
             !g_strcmp0(adw_about_dialog_get_version(a), "2.1") &&
             !g_strcmp0(adw_about_dialog_get_developer_name(a), "QNX Porters"), "metainfo: name, newest version, developer");
    QT_CHECK(adw_about_dialog_get_license_type(a) == GTK_LICENSE_APACHE_2_0 &&
             !g_strcmp0(adw_about_dialog_get_issue_url(a), "https://example.org/qnx/issues"), "metainfo: license type and issue URL");
    QT_CHECK(adw_about_dialog_get_release_notes(a) && strstr(adw_about_dialog_get_release_notes(a), "Second."), "metainfo: release notes of 2.1");
    g_object_ref_sink(ad); g_object_unref(ad);
  }
  gtk_window_destroy(GTK_WINDOW(win));
  spin(100);
  n10_stop();
  unlink(path); rmdir(dir);
  QT_END();
}
