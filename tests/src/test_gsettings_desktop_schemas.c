/* gsettings-desktop-schemas: compile the installed schemas with
 * glib-compile-schemas (--strict) into a private directory, load them with
 * GSettings (memory backend) as GNOME/GTK applications do, read documented
 * defaults, use the enum header (gdesktop-enums.h) to set an enum key and
 * read its nick back, check a range, a relocatable schema, and -- with the
 * gsettings tool -- read a key from the command line. */
#define G_SETTINGS_ENABLE_BACKEND 1
#include "common.h"
#include <gio/gio.h>
#include <gio/gsettingsbackend.h>
#include <glib/gstdio.h>
#include <gdesktop-enums.h>
#include <stdlib.h>
#include <unistd.h>

static char *tool(const char *name) {
  const char *dirs[] = { "/usr/local/bin", "/data/oss/usr/local/bin" };
  for (int i = 0; i < 2; i++) { char *p = g_strdup_printf("%s/%s", dirs[i], name); if (access(p, X_OK) == 0) return p; g_free(p); }
  return g_find_program_in_path(name);
}
static void rmtree(const char *d) {
  GDir *dir = g_dir_open(d, 0, NULL); const char *n;
  while (dir && (n = g_dir_read_name(dir))) { char *p = g_build_filename(d, n, NULL); g_unlink(p); g_free(p); }
  if (dir) g_dir_close(dir);
  g_rmdir(d);
}

int main(void) {
  QT_BEGIN("gsettings-desktop-schemas");
  const char *roots[] = { getenv("QNX_PORT_DATA_ROOT"), "/usr/local/share", "/data/oss/usr/local/share" };
  char *src = NULL;
  for (int i = 0; i < 3 && !src; i++) if (roots[i]) {
    char *p = g_strdup_printf("%s/glib-2.0/schemas", roots[i]), *f = g_strdup_printf("%s/org.gnome.desktop.interface.gschema.xml", p);
    if (access(f, R_OK) == 0) src = p; else g_free(p);
    g_free(f);
  }
  QT_CHECK(src != NULL, "share/glib-2.0/schemas/org.gnome.desktop.*.gschema.xml installed");
  char *gcs = tool("glib-compile-schemas");
  QT_CHECK(gcs != NULL, "glib-compile-schemas available");
  if (!src || !gcs) QT_END();
  GError *err = NULL;
  char *tmp = g_dir_make_tmp("qnxport-gsettings-XXXXXX", &err);
  /* only this package's files: other packages' schemas share the directory */
  char *own = g_build_filename(tmp, "src", NULL); g_mkdir(own, 0700);
  GDir *sd = g_dir_open(src, 0, NULL); const char *fn; int copied = 0;
  while (sd && (fn = g_dir_read_name(sd))) {
    if (strncmp(fn, "org.gnome.desktop.", 18) && strncmp(fn, "org.gnome.system.", 17)) continue;
    char *from = g_build_filename(src, fn, NULL), *to = g_build_filename(own, fn, NULL), *data = NULL; gsize len = 0;
    if (g_file_get_contents(from, &data, &len, NULL) && g_file_set_contents(to, data, len, NULL)) copied++;
    g_free(from); g_free(to); g_free(data);
  }
  if (sd) g_dir_close(sd);
  printf("  info %d schema files of the package\n", copied);
  char *targ = g_strdup_printf("--targetdir=%s", tmp);
  char *argv[] = { gcs, "--strict", targ, own, NULL };
  char *out = NULL, *errout = NULL; int status = -1;
  gboolean ran = g_spawn_sync(NULL, argv, NULL, 0, NULL, NULL, &out, &errout, &status, &err);
  if (errout && *errout) printf("  info glib-compile-schemas: %s", errout);
  char *compiled = g_build_filename(tmp, "gschemas.compiled", NULL);
  QT_CHECK(ran && g_spawn_check_wait_status(status, NULL) && access(compiled, R_OK) == 0, "glib-compile-schemas --strict compiled them");
  GSettingsSchemaSource *ss = g_settings_schema_source_new_from_directory(tmp, NULL, TRUE, &err);
  QT_CHECK(ss != NULL, "schema source loads gschemas.compiled");
  if (!ss) { rmtree(own); rmtree(tmp); QT_END(); }
  char **nonreloc = NULL, **reloc = NULL;
  g_settings_schema_source_list_schemas(ss, FALSE, &nonreloc, &reloc);
  printf("  info %u schemas, %u relocatable\n", g_strv_length(nonreloc), g_strv_length(reloc));
  QT_CHECK(g_strv_length(nonreloc) >= 30 && g_strv_contains((const char *const *)nonreloc, "org.gnome.desktop.wm.preferences")
           && g_strv_contains((const char *const *)nonreloc, "org.gnome.system.proxy"), "30+ schemas, wm.preferences and system.proxy among them");
  QT_CHECK(g_strv_contains((const char *const *)reloc, "org.gnome.desktop.app-folders.folder"), "relocatable app-folders.folder");

  GSettingsSchema *iface = g_settings_schema_source_lookup(ss, "org.gnome.desktop.interface", FALSE);
  GSettingsBackend *mem = g_memory_settings_backend_new();
  GSettings *s = iface ? g_settings_new_full(iface, mem, NULL) : NULL;
  QT_CHECK(s != NULL, "GSettings for org.gnome.desktop.interface (memory backend)");
  if (s) {
    char *font = g_settings_get_string(s, "font-name"), *theme = g_settings_get_string(s, "gtk-theme");
    printf("  info font-name '%s', gtk-theme '%s'\n", font, theme);
    QT_CHECK(!strcmp(font, "Adwaita Sans 11") && !strcmp(theme, "Adwaita"), "defaults: font-name 'Adwaita Sans 11', gtk-theme 'Adwaita'");
    QT_CHECK(g_settings_get_enum(s, "clock-format") == G_DESKTOP_CLOCK_FORMAT_24H && g_settings_get_enum(s, "color-scheme") == G_DESKTOP_COLOR_SCHEME_DEFAULT,
             "enum defaults match gdesktop-enums.h: 24h, default");
    QT_CHECK(g_settings_set_enum(s, "color-scheme", G_DESKTOP_COLOR_SCHEME_PREFER_DARK), "set color-scheme = G_DESKTOP_COLOR_SCHEME_PREFER_DARK");
    char *cs = g_settings_get_string(s, "color-scheme");
    QT_CHECK(!strcmp(cs, "prefer-dark"), "reads back as nick 'prefer-dark'");
    GSettingsSchemaKey *k = g_settings_schema_get_key(iface, "color-scheme");
    GVariant *bad = g_variant_new_string("neon");
    QT_CHECK(!g_settings_schema_key_range_check(k, bad), "'neon' is not a valid color-scheme");
    g_variant_unref(g_variant_ref_sink(bad));
    g_settings_schema_key_unref(k);
    g_free(font); g_free(theme); g_free(cs);
    g_object_unref(s);
  }
  GSettingsSchema *wm = g_settings_schema_source_lookup(ss, "org.gnome.desktop.wm.preferences", FALSE);
  if (wm) {
    GSettingsSchemaKey *k = g_settings_schema_get_key(wm, "num-workspaces");
    GVariant *range = g_settings_schema_key_get_range(k);
    const char *type; GVariant *detail; int lo = 0, hi = 0;
    g_variant_get(range, "(&sv)", &type, &detail);
    if (!strcmp(type, "range")) g_variant_get(detail, "(ii)", &lo, &hi);
    QT_CHECK(!strcmp(type, "range") && lo == 1 && hi == 36, "wm num-workspaces: range 1..36");
    GSettings *w = g_settings_new_full(wm, mem, NULL);
    QT_CHECK(g_settings_get_int(w, "num-workspaces") == 4, "num-workspaces default 4");
    g_variant_unref(detail); g_variant_unref(range); g_settings_schema_key_unref(k); g_object_unref(w);
    g_settings_schema_unref(wm);
  } else QT_CHECK(0, "org.gnome.desktop.wm.preferences");

  char *gs = tool("gsettings");
  if (gs) {
    char **env = g_get_environ();
    env = g_environ_setenv(env, "GSETTINGS_SCHEMA_DIR", tmp, TRUE);
    env = g_environ_setenv(env, "GSETTINGS_BACKEND", "memory", TRUE);
    char *gargv[] = { gs, "get", "org.gnome.desktop.interface", "font-name", NULL };
    char *o2 = NULL, *e2 = NULL; int st2 = -1;
    gboolean r2 = g_spawn_sync(NULL, gargv, env, 0, NULL, NULL, &o2, &e2, &st2, NULL);
    printf("  info gsettings get: %s%s", o2 ? o2 : "", e2 && *e2 ? e2 : "");
    QT_CHECK(r2 && g_spawn_check_wait_status(st2, NULL) && o2 && !strcmp(o2, "'Adwaita Sans 11'\n"), "gsettings get org.gnome.desktop.interface font-name");
    g_strfreev(env); g_free(o2); g_free(e2); g_free(gs);
  } else printf("  info no gsettings tool: command line not exercised\n");
  if (iface) g_settings_schema_unref(iface);
  g_object_unref(mem);
  g_settings_schema_source_unref(ss);
  g_strfreev(nonreloc); g_strfreev(reloc);
  rmtree(own);
  rmtree(tmp);
  QT_END();
}
