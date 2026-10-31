/* garcon: freedesktop.org desktop entries and menus -- a .desktop file parsed
 * into a GarconMenuItem (name, command, categories, keywords, actions,
 * terminal/no-display flags), and a .menu file with a sub-menu, AppDir and
 * category rules loaded into a menu tree with the expected items. */
#include "common.h"
#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>
#include <garcon/garcon.h>

static void put(const char *path, const char *text) {
  FILE *f = fopen(path, "w");
  if (f) { fputs(text, f); fclose(f); }
}
static int has_item(GarconMenu *m, const char *name) {
  GList *items = garcon_menu_get_items(m);
  int found = 0;
  for (GList *l = items; l; l = l->next)
    if (g_strcmp0(garcon_menu_item_get_name(GARCON_MENU_ITEM(l->data)), name) == 0) found = 1;
  g_list_free(items);
  return found;
}

int main(void) {
  QT_BEGIN("garcon");
  printf("  info garcon %u.%u.%u\n", garcon_major_version, garcon_minor_version, garcon_micro_version);
  const char *d = "/tmp/qnxport-garcon", *apps = "/tmp/qnxport-garcon/apps";
  mkdir(d, 0755); mkdir(apps, 0755);
  setenv("XDG_CONFIG_HOME", d, 1); setenv("XDG_DATA_HOME", d, 1);
  setenv("XDG_CONFIG_DIRS", d, 1); setenv("XDG_DATA_DIRS", d, 1);
  put("/tmp/qnxport-garcon/apps/editor.desktop",
      "[Desktop Entry]\nType=Application\nName=QNX Editor\nGenericName=Text Editor\nComment=Edit text files\n"
      "Exec=qed %F\nIcon=accessories-text-editor\nTerminal=false\nCategories=Development;TextEditor;\n"
      "Keywords=text;code;\nActions=new-window;\n\n[Desktop Action new-window]\nName=New Window\nExec=qed --new\n");
  put("/tmp/qnxport-garcon/apps/game.desktop",
      "[Desktop Entry]\nType=Application\nName=Neutrino Tetris\nExec=ntetris\nTerminal=true\nCategories=Game;\n");
  put("/tmp/qnxport-garcon/apps/tool.desktop",
      "[Desktop Entry]\nType=Application\nName=Hidden Tool\nExec=htool\nNoDisplay=true\nCategories=Utility;\n");
  put("/tmp/qnxport-garcon/test.menu",
      "<!DOCTYPE Menu PUBLIC \"-//freedesktop//DTD Menu 1.0//EN\" \"http://www.freedesktop.org/standards/menu-spec/1.0/menu.dtd\">\n"
      "<Menu><Name>Root</Name><AppDir>/tmp/qnxport-garcon/apps</AppDir>\n"
      " <Include><Category>Development</Category></Include>\n"
      " <Menu><Name>Games</Name><Include><Category>Game</Category></Include></Menu>\n"
      "</Menu>\n");

  GarconMenuItem *it = garcon_menu_item_new_for_path("/tmp/qnxport-garcon/apps/editor.desktop");
  QT_CHECK(it != NULL, "desktop file parsed");
  if (it) {
    QT_CHECK(g_strcmp0(garcon_menu_item_get_name(it), "QNX Editor") == 0 &&
             g_strcmp0(garcon_menu_item_get_generic_name(it), "Text Editor") == 0, "Name / GenericName");
    QT_CHECK(g_strcmp0(garcon_menu_item_get_command(it), "qed %F") == 0 &&
             g_strcmp0(garcon_menu_item_get_icon_name(it), "accessories-text-editor") == 0, "Exec / Icon");
    QT_CHECK(garcon_menu_item_has_category(it, "TextEditor") && !garcon_menu_item_has_category(it, "Game"), "Categories");
    QT_CHECK(garcon_menu_item_has_keyword(it, "code"), "Keywords");
    QT_CHECK(!garcon_menu_item_requires_terminal(it) && !garcon_menu_item_get_no_display(it), "Terminal=false, displayed");
    /* garcon looks actions up by their Name= (the display name), not by the
       [Desktop Action <id>] id. */
    GarconMenuItemAction *act = garcon_menu_item_get_action(it, "New Window");
    QT_CHECK(act && g_strcmp0(garcon_menu_item_action_get_command(act), "qed --new") == 0, "desktop action parsed");
    g_object_unref(it);
  }
  GarconMenuItem *tool = garcon_menu_item_new_for_path("/tmp/qnxport-garcon/apps/tool.desktop");
  QT_CHECK(tool && garcon_menu_item_get_no_display(tool), "NoDisplay=true honoured");
  if (tool) g_object_unref(tool);

  GarconMenu *menu = garcon_menu_new_for_path("/tmp/qnxport-garcon/test.menu");
  GError *err = NULL;
  gboolean ok = menu && garcon_menu_load(menu, NULL, &err);
  if (err) { printf("  info %s\n", err->message); g_error_free(err); }
  QT_CHECK(ok, "menu file loaded");
  if (ok) {
    QT_CHECK(has_item(menu, "QNX Editor") && !has_item(menu, "Neutrino Tetris"), "root menu holds the Development item only");
    GList *subs = garcon_menu_get_menus(menu);
    GarconMenu *games = subs ? GARCON_MENU(subs->data) : NULL;
    QT_CHECK(g_list_length(subs) == 1 && games && g_strcmp0(garcon_menu_element_get_name(GARCON_MENU_ELEMENT(games)), "Games") == 0,
             "one sub-menu named Games");
    QT_CHECK(games && has_item(games, "Neutrino Tetris") && !has_item(games, "QNX Editor"), "Games holds the Game item");
    g_list_free(subs);
  }
  if (menu) g_object_unref(menu);
  unlink("/tmp/qnxport-garcon/apps/editor.desktop"); unlink("/tmp/qnxport-garcon/apps/game.desktop");
  unlink("/tmp/qnxport-garcon/apps/tool.desktop"); unlink("/tmp/qnxport-garcon/test.menu");
  rmdir(apps); rmdir(d);
  QT_END();
}
