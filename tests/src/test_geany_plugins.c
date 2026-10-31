/* geany-plugins: the shared plugin utility library (libgeanypluginutils:
 * project file-list scanning with include/ignore patterns, symlink-loop
 * protection, path matching), and every installed plugin module (Geany's
 * own and geany-plugins') loads and exports a plugin entry point. The
 * plugins themselves need a running Geany window, which a headless target
 * does not have. */
#include "common.h"
#include <dirent.h>
#include <dlfcn.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>
#include <glib.h>

/* libgeanypluginutils ships no header; these are utils/src/filelist.h. */
enum { FILELIST_FLAG_ADD_DIRS = 1 };
GSList *gp_filelist_scan_directory(guint *files, guint *folders, const gchar *searchdir, gchar **file_patterns,
                                   gchar **ignored_dirs_patterns, gchar **ignored_file_patterns);
GSList *gp_filelist_scan_directory_full(guint *files, guint *folders, const gchar *searchdir, gchar **file_patterns,
                                        gchar **ignored_dirs_patterns, gchar **ignored_file_patterns, guint flags);
gboolean gp_filelist_filepath_matches_patterns(const gchar *filepath, gchar **file_patterns,
                                               gchar **ignored_dirs_patterns, gchar **ignored_file_patterns);

static void touch(const char *p) { FILE *f = fopen(p, "w"); if (f) { fputs("x\n", f); fclose(f); } }
static int in_list(GSList *l, const char *path) {
  for (; l; l = l->next) if (strcmp((const char *)l->data, path) == 0) return 1;
  return 0;
}

int main(void) {
  QT_BEGIN("geany-plugins");
  const char *R = "/tmp/qnxport-geany";
  mkdir(R, 0755); mkdir("/tmp/qnxport-geany/sub", 0755); mkdir("/tmp/qnxport-geany/build", 0755);
  touch("/tmp/qnxport-geany/main.c"); touch("/tmp/qnxport-geany/main.h"); touch("/tmp/qnxport-geany/README");
  touch("/tmp/qnxport-geany/backup.c"); touch("/tmp/qnxport-geany/sub/util.c"); touch("/tmp/qnxport-geany/build/gen.c");
  symlink("..", "/tmp/qnxport-geany/sub/loop");   /* must not be followed forever */

  gchar *pats[] = {"*.c", "*.h", NULL}, *idirs[] = {"build", NULL}, *ifiles[] = {"backup*", NULL};
  guint files = 0, folders = 0;
  GSList *l = gp_filelist_scan_directory(&files, &folders, R, pats, idirs, ifiles);
  printf("  info %u file(s), %u folder(s)\n", files, folders);
  QT_CHECK(files == 3 && g_slist_length(l) == 3, "three matching files found");
  QT_CHECK(in_list(l, "/tmp/qnxport-geany/main.c") && in_list(l, "/tmp/qnxport-geany/main.h") &&
           in_list(l, "/tmp/qnxport-geany/sub/util.c"), "main.c, main.h and sub/util.c listed");
  QT_CHECK(!in_list(l, "/tmp/qnxport-geany/build/gen.c") && !in_list(l, "/tmp/qnxport-geany/backup.c") &&
           !in_list(l, "/tmp/qnxport-geany/README"), "ignored directory, ignored file and non-matching file left out");
  g_slist_free_full(l, g_free);

  l = gp_filelist_scan_directory_full(&files, &folders, R, NULL, idirs, NULL, FILELIST_FLAG_ADD_DIRS);
  QT_CHECK(in_list(l, "/tmp/qnxport-geany/sub") && in_list(l, "/tmp/qnxport-geany/README") && files == 5,
           "ADD_DIRS lists directories too; NULL pattern matches every file");
  g_slist_free_full(l, g_free);

  /* Unlike the scanner (which matches base names), this matches the patterns
     against the whole path, so the ignore pattern needs a leading '*'. */
  gchar *ifiles_path[] = {"*/backup*", NULL};
  QT_CHECK(gp_filelist_filepath_matches_patterns("/tmp/qnxport-geany/sub/util.c", pats, idirs, ifiles_path) &&
           !gp_filelist_filepath_matches_patterns("/tmp/qnxport-geany/README", pats, idirs, ifiles_path) &&
           !gp_filelist_filepath_matches_patterns("/tmp/qnxport-geany/backup.c", pats, idirs, ifiles_path),
           "single-path pattern check (whole-path patterns)");

  unlink("/tmp/qnxport-geany/sub/loop");
  const char *rm[] = {"main.c", "main.h", "README", "backup.c", "sub/util.c", "build/gen.c"};
  for (size_t i = 0; i < sizeof rm / sizeof *rm; ++i) {
    char p[128]; snprintf(p, sizeof p, "%s/%s", R, rm[i]); unlink(p);
  }
  rmdir("/tmp/qnxport-geany/sub"); rmdir("/tmp/qnxport-geany/build"); rmdir(R);

  const char *pdir = "/usr/local/lib/geany";
  DIR *dd = opendir(pdir);
  int total = 0, loaded = 0;
  if (dd) {
    struct dirent *e;
    while ((e = readdir(dd)) != NULL) {
      size_t n = strlen(e->d_name);
      if (n < 4 || strcmp(e->d_name + n - 3, ".so") != 0) continue;
      char p[512]; snprintf(p, sizeof p, "%s/%s", pdir, e->d_name);
      ++total;
      void *h = dlopen(p, RTLD_NOW | RTLD_LOCAL);
      /* Two plugin ABIs: geany_load_module() (Geany >= 1.26), or the legacy
         plugin_version_check() + plugin_init() pair many plugins still use. */
      if (h && (dlsym(h, "geany_load_module") || (dlsym(h, "plugin_version_check") && dlsym(h, "plugin_init")))) ++loaded;
      else printf("  info %s: %s\n", e->d_name, h ? "no plugin entry point" : dlerror());
      /* not dlclose'd: GTK type registrations do not survive unloading */
    }
    closedir(dd);
  }
  printf("  info %d of %d plugin module(s) loaded\n", loaded, total);
  QT_CHECK(total >= 10 && loaded == total, "every plugin in /usr/local/lib/geany loads and exports a plugin entry point");
  QT_END();
}
