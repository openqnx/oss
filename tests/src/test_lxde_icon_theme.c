/* lxde-icon-theme (nuoveXT2): the theme as an icon loader sees it -- its
 * index.theme parses (GKeyFile), names the theme and its parents, and every
 * directory it lists has its section and exists; icons are looked up the
 * freedesktop way (theme dir, size, context) and loaded with gdk-pixbuf at
 * the size their directory promises; symlinked icons (user-home ->
 * go-home) resolve; no symlink in the theme dangles. */
#include "common.h"
#include <dirent.h>
#include <gdk-pixbuf/gdk-pixbuf.h>
#include <glib.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>

static int nlinks, dangling, npng;
static void walk(const char *dir) {
  DIR *d = opendir(dir); struct dirent *e;
  if (!d) return;
  while ((e = readdir(d))) {
    if (e->d_name[0] == '.') continue;
    char p[1024]; struct stat ls, st; snprintf(p, sizeof p, "%s/%s", dir, e->d_name);
    if (lstat(p, &ls)) continue;
    if (S_ISLNK(ls.st_mode)) { nlinks++; if (stat(p, &st)) { if (dangling++ < 3) printf("  info dangling: %s\n", p); } }
    else if (S_ISDIR(ls.st_mode)) { walk(p); continue; }
    if (strstr(e->d_name, ".png")) npng++;
  }
  closedir(d);
}

/* freedesktop lookup in one theme: the directory of this size and context */
static char *lookup(GKeyFile *kf, const char *root, const char *icon, int size, const char *context) {
  gsize n = 0; char **dirs = g_key_file_get_string_list(kf, "Icon Theme", "Directories", &n, NULL);
  char *found = NULL;
  for (gsize i = 0; i < n && !found; i++) {
    if (g_key_file_get_integer(kf, dirs[i], "Size", NULL) != size) continue;
    char *ctx = g_key_file_get_string(kf, dirs[i], "Context", NULL);
    if (ctx && !strcmp(ctx, context)) {
      char *p = g_strdup_printf("%s/%s/%s.png", root, dirs[i], icon);
      if (access(p, R_OK) == 0) found = p; else g_free(p);
    }
    g_free(ctx);
  }
  g_strfreev(dirs);
  return found;
}

int main(void) {
  QT_BEGIN("lxde-icon-theme");
  const char *roots[] = { getenv("QNX_PORT_DATA_ROOT"), "/usr/local/share", "/data/oss/usr/local/share" };
  char root[512] = "";
  for (int i = 0; i < 3 && !root[0]; i++) if (roots[i]) {
    char p[600]; snprintf(p, sizeof p, "%s/icons/nuoveXT2/index.theme", roots[i]);
    if (access(p, R_OK) == 0) snprintf(root, sizeof root, "%s/icons/nuoveXT2", roots[i]);
  }
  QT_CHECK(root[0], "share/icons/nuoveXT2/index.theme installed");
  if (!root[0]) QT_END();
  char *idx = g_strdup_printf("%s/index.theme", root);
  GKeyFile *kf = g_key_file_new(); GError *err = NULL;
  g_key_file_set_list_separator(kf, ',');       /* icon themes separate lists with commas */
  gboolean ok = g_key_file_load_from_file(kf, idx, G_KEY_FILE_NONE, &err);
  if (!ok) printf("  info %s\n", err->message);
  QT_CHECK(ok, "index.theme parses as a key file");
  if (!ok) QT_END();
  char *name = g_key_file_get_string(kf, "Icon Theme", "Name", NULL), *inh = g_key_file_get_string(kf, "Icon Theme", "Inherits", NULL);
  printf("  info Name=%s Inherits=%s\n", name, inh);
  QT_CHECK(name && !strcmp(name, "nuoveXT.2.2") && inh && strstr(inh, "gnome"), "Name nuoveXT.2.2, inherits gnome");
  gsize n = 0; char **dirs = g_key_file_get_string_list(kf, "Icon Theme", "Directories", &n, NULL);
  int sections = 0, exist = 0, sizes_ok = 0;
  for (gsize i = 0; i < n; i++) {
    if (g_key_file_has_group(kf, dirs[i])) sections++;
    char *p = g_strdup_printf("%s/%s", root, dirs[i]); exist += g_file_test(p, G_FILE_TEST_IS_DIR); g_free(p);
    int sz = g_key_file_get_integer(kf, dirs[i], "Size", NULL);
    sizes_ok += atoi(dirs[i]) == sz;
  }
  printf("  info %zu directories listed, %d sections, %d present\n", (size_t)n, sections, exist);
  QT_CHECK(n >= 90 && sections == (int)n && exist == (int)n, "every listed directory has a section and exists");
  QT_CHECK(sizes_ok == (int)n, "each directory's Size matches its name (16x16 -> 16 ...)");
  g_strfreev(dirs);

  struct { const char *icon, *ctx; int size; } want[] = {
    { "folder", "Places", 48 }, { "folder", "Places", 16 }, { "user-trash", "Places", 48 }, { "document-open", "Actions", 24 },
    { "utilities-terminal", "Applications", 32 }, { "computer", "Devices", 128 }, { "dialog-warning", "Status", 22 },
    { "user-home", "Places", 48 } };
  for (size_t i = 0; i < sizeof want / sizeof want[0]; i++) {
    char *p = lookup(kf, root, want[i].icon, want[i].size, want[i].ctx);
    GError *e2 = NULL;
    GdkPixbuf *pb = p ? gdk_pixbuf_new_from_file(p, &e2) : NULL;
    char what[200];
    snprintf(what, sizeof what, "%s (%s, %d): found, loads as %dx%d%s", want[i].icon, want[i].ctx, want[i].size,
             pb ? gdk_pixbuf_get_width(pb) : 0, pb ? gdk_pixbuf_get_height(pb) : 0, pb && gdk_pixbuf_get_has_alpha(pb) ? " RGBA" : "");
    QT_CHECK(pb && gdk_pixbuf_get_width(pb) == want[i].size && gdk_pixbuf_get_height(pb) == want[i].size, what);
    if (e2) { printf("  info %s\n", e2->message); g_error_free(e2); }
    if (pb) g_object_unref(pb);
    g_free(p);
  }
  walk(root);
  printf("  info %d PNG icons, %d symlinks, %d dangling\n", npng, nlinks, dangling);
  QT_CHECK(npng > 6000 && dangling == 0, "6000+ PNG icons, no dangling symlink");
  g_key_file_free(kf);
  g_free(idx);
  QT_END();
}
