/* hicolor-icon-theme: the fallback theme every icon theme inherits, which
 * ships only its index.theme -- the directory layout applications install
 * their icons into. It must parse (GKeyFile) and describe that layout:
 * hidden fallback theme, the standard sizes and contexts, HiDPI (@2)
 * directories with Scale=2, scalable directories with their size range,
 * and a section for every directory it lists. */
#include "common.h"
#include <glib.h>
#include <stdlib.h>
#include <unistd.h>

int main(void) {
  QT_BEGIN("hicolor-icon-theme");
  const char *roots[] = { getenv("QNX_PORT_DATA_ROOT"), "/usr/local/share", "/data/oss/usr/local/share" };
  char *idx = NULL;
  for (int i = 0; i < 3 && !idx; i++) if (roots[i]) {
    char *p = g_strdup_printf("%s/icons/hicolor/index.theme", roots[i]);
    if (access(p, R_OK) == 0) idx = p; else g_free(p);
  }
  QT_CHECK(idx != NULL, "share/icons/hicolor/index.theme installed");
  if (!idx) QT_END();
  GKeyFile *kf = g_key_file_new(); GError *err = NULL;
  g_key_file_set_list_separator(kf, ',');       /* icon themes separate lists with commas */
  gboolean ok = g_key_file_load_from_file(kf, idx, G_KEY_FILE_NONE, &err);
  if (!ok) printf("  info %s\n", err->message);
  QT_CHECK(ok, "index.theme parses as a key file");
  if (!ok) QT_END();
  char *name = g_key_file_get_string(kf, "Icon Theme", "Name", NULL);
  QT_CHECK(name && !strcmp(name, "Hicolor") && g_key_file_get_boolean(kf, "Icon Theme", "Hidden", NULL), "Name=Hicolor, Hidden=true");
  gsize n = 0; char **dirs = g_key_file_get_string_list(kf, "Icon Theme", "Directories", &n, NULL);
  int sections = 0, scaled = 0, scaled_ok = 0, scalable = 0, contexts_ok = 0;
  for (gsize i = 0; i < n; i++) {
    if (!g_key_file_has_group(kf, dirs[i])) continue;
    sections++;
    char *ctx = g_key_file_get_string(kf, dirs[i], "Context", NULL);
    char *type = g_key_file_get_string(kf, dirs[i], "Type", NULL);
    int size = g_key_file_get_integer(kf, dirs[i], "Size", NULL);
    contexts_ok += ctx && ctx[0] && size > 0;
    if (strchr(dirs[i], '@')) { scaled++; scaled_ok += g_key_file_get_integer(kf, dirs[i], "Scale", NULL) == 2 && size == atoi(dirs[i]); }
    if (!strncmp(dirs[i], "scalable/", 9)) scalable += type && !strcmp(type, "Scalable") && g_key_file_get_integer(kf, dirs[i], "MaxSize", NULL) >= 256;
    g_free(ctx); g_free(type);
  }
  printf("  info %zu directories, %d sections, %d HiDPI, %d scalable\n", (size_t)n, sections, scaled, scalable);
  QT_CHECK(n > 300 && sections == (int)n, "300+ directories, each with its section");
  QT_CHECK(contexts_ok == (int)n, "every section has a Context and a Size");
  QT_CHECK(scaled > 100 && scaled_ok == scaled, "@2 directories have Scale=2 and the base size");
  QT_CHECK(scalable >= 20, "scalable/* directories: Type=Scalable, MaxSize >= 256");
  struct { const char *dir, *ctx; int size; } k[] = { { "48x48/apps", "Applications", 48 }, { "16x16/mimetypes", "MimeTypes", 16 },
                                                       { "256x256/places", "Places", 256 }, { "symbolic/apps", "Applications", 16 } };
  int good = 0;
  for (int i = 0; i < 4; i++) {
    char *ctx = g_key_file_get_string(kf, k[i].dir, "Context", NULL);
    good += ctx && !strcmp(ctx, k[i].ctx) && g_key_file_get_integer(kf, k[i].dir, "Size", NULL) == k[i].size;
    g_free(ctx);
  }
  QT_CHECK(good == 4, "48x48/apps, 16x16/mimetypes, 256x256/places, symbolic/apps as expected");
  g_strfreev(dirs);
  g_key_file_free(kf);
  g_free(idx);
  QT_END();
}
