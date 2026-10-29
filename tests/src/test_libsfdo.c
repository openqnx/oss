#include "common.h"
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sfdo-basedir.h>
#include <sfdo-desktop.h>
/* A desktop entry database read from a directory made here: one application
 * entry with a localized name and categories, one hidden entry. Known
 * answers: the id, the localized name for de, the Exec line, the hidden one
 * flagged NoDisplay. */
int main(void) {
  QT_BEGIN("libsfdo");
  char base[] = "/tmp/sfdo-XXXXXX";
  QT_CHECK(mkdtemp(base) != NULL, "temporary data directory");
  char apps[256], f1[300], f2[300];
  snprintf(apps, sizeof apps, "%s/applications", base); mkdir(apps, 0755);
  snprintf(f1, sizeof f1, "%s/qnx-demo.desktop", apps);
  snprintf(f2, sizeof f2, "%s/qnx-hidden.desktop", apps);
  FILE *f = fopen(f1, "w");
  fputs("[Desktop Entry]\nType=Application\nName=QNX Demo\nName[de]=QNX Vorfuehrung\n"
        "Exec=qnx-demo --flag %f\nIcon=utilities-terminal\nCategories=System;Utility;\n", f); fclose(f);
  f = fopen(f2, "w");
  fputs("[Desktop Entry]\nType=Application\nName=Hidden\nExec=true\nNoDisplay=true\n", f); fclose(f);

  struct sfdo_basedir_ctx *bd = sfdo_basedir_ctx_create();
  QT_CHECK(bd != NULL, "sfdo_basedir_ctx_create");
  struct sfdo_desktop_ctx *ctx = sfdo_desktop_ctx_create(bd);
  QT_CHECK(ctx != NULL, "sfdo_desktop_ctx_create");
  /* load_from takes the applications directories themselves (load adds
   * "applications/" to each XDG data directory). */
  char dir[300]; snprintf(dir, sizeof dir, "%s/", apps);
  struct sfdo_string bds[1] = { { dir, strlen(dir) } };
  struct sfdo_desktop_db *db = sfdo_desktop_db_load_from(ctx, "de_DE.UTF-8", bds, 1);
  QT_CHECK(db != NULL, "sfdo_desktop_db_load_from");
  if (db) {
    size_t n = 0; struct sfdo_desktop_entry **all = sfdo_desktop_db_get_entries(db, &n);
    printf("  info %zu entries\n", n);
    for (size_t i = 0; i < n; i++) { size_t l; printf("  info id %s\n", sfdo_desktop_entry_get_id(all[i], &l)); }
    QT_CHECK(n == 2, "two entries found");
    struct sfdo_desktop_entry *e = sfdo_desktop_db_get_entry_by_id(db, "qnx-demo", SFDO_NT);
    QT_CHECK(e != NULL, "entry qnx-demo by id");
    if (e) {
      size_t l; const char *name = sfdo_desktop_entry_get_name(e, &l);
      printf("  info name: %s\n", name);
      QT_CHECK(name && strcmp(name, "QNX Vorfuehrung") == 0, "localized name (de)");
      QT_CHECK(sfdo_desktop_entry_get_type(e) == SFDO_DESKTOP_ENTRY_APPLICATION, "type Application");
      QT_CHECK(sfdo_desktop_entry_get_exec(e) != NULL, "Exec parsed");
      const char *icon = sfdo_desktop_entry_get_icon(e, &l);
      QT_CHECK(icon && strcmp(icon, "utilities-terminal") == 0, "Icon");
      size_t nc = 0; sfdo_desktop_entry_get_categories(e, &nc);
      QT_CHECK(nc == 2, "two categories");
    }
    struct sfdo_desktop_entry *h = sfdo_desktop_db_get_entry_by_id(db, "qnx-hidden", SFDO_NT);
    QT_CHECK(h && sfdo_desktop_entry_get_no_display(h), "NoDisplay entry flagged");
    sfdo_desktop_db_destroy(db);
  }
  sfdo_desktop_ctx_destroy(ctx); sfdo_basedir_ctx_destroy(bd);
  unlink(f1); unlink(f2); rmdir(apps); rmdir(base);
  QT_END();
}
