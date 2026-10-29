/* GLib: the pieces a port most often breaks -- the main loop (which sits on
 * QNX's epoll shim), threads, timers, IO channels, GObject type registration,
 * and the filesystem/UTF-8 helpers. */
#include "common.h"
#include <glib.h>
#include <glib-object.h>
#include <gio/gio.h>
static int timeout_fired, idle_fired;
static gboolean on_timeout(gpointer data) {
  ++timeout_fired;
  g_main_loop_quit((GMainLoop *)data);
  return G_SOURCE_REMOVE;
}
static gboolean on_idle(gpointer data) { ++idle_fired; return G_SOURCE_REMOVE; }
static gpointer worker(gpointer data) {
  g_atomic_int_inc((gint *)data);
  return GINT_TO_POINTER(42);
}
int main(void) {
  QT_BEGIN("glib");
  printf("  info glib %u.%u.%u\n", glib_major_version, glib_minor_version,
         glib_micro_version);
  QT_CHECK(glib_major_version == 2, "GLib 2.x");

  /* --- main loop, timers and idles (epoll/timerfd shims underneath) --- */
  GMainContext *ctx = g_main_context_default();
  QT_CHECK(ctx != NULL, "default main context");
  GMainLoop *loop = g_main_loop_new(ctx, FALSE);
  g_idle_add(on_idle, NULL);
  g_timeout_add(30, on_timeout, loop);
  g_main_loop_run(loop);
  QT_CHECK(idle_fired == 1, "idle source ran");
  QT_CHECK(timeout_fired == 1, "timeout source ran and quit the loop");
  g_main_loop_unref(loop);

  /* --- threads --- */
  gint counter = 0;
  GThread *t = g_thread_new("qnx-port-test", worker, &counter);
  QT_CHECK(t != NULL, "g_thread_new");
  QT_CHECK(GPOINTER_TO_INT(g_thread_join(t)) == 42, "thread returned its value");
  QT_CHECK(g_atomic_int_get(&counter) == 1, "thread ran");

  /* --- GObject type system --- */
  GObject *obj = g_object_new(G_TYPE_OBJECT, NULL);
  QT_CHECK(obj != NULL, "g_object_new");
  QT_CHECK(G_IS_OBJECT(obj), "G_IS_OBJECT");
  g_object_unref(obj);
  QT_CHECK(g_type_from_name("GObject") == G_TYPE_OBJECT, "g_type_from_name");

  /* --- strings, UTF-8 and collections --- */
  gchar *up = g_utf8_strup("stra\xc3\x9f" "e", -1);
  QT_CHECK(up && g_utf8_strlen(up, -1) >= 6, "g_utf8_strup");
  g_free(up);
  GString *s = g_string_new("sdp=");
  g_string_append_printf(s, "%d", 8);
  QT_CHECK(g_strcmp0(s->str, "sdp=8") == 0, "GString formatting");
  g_string_free(s, TRUE);
  GHashTable *h = g_hash_table_new(g_str_hash, g_str_equal);
  g_hash_table_insert(h, (gpointer) "arch", (gpointer) "aarch64le");
  QT_CHECK(g_hash_table_size(h) == 1, "GHashTable insert");
  QT_CHECK(g_strcmp0(g_hash_table_lookup(h, "arch"), "aarch64le") == 0,
           "GHashTable lookup");
  g_hash_table_destroy(h);

  /* --- GIO: the file layer, which needs QNX's mount table support --- */
  GFile *f = g_file_new_for_path("/tmp");
  QT_CHECK(f != NULL, "g_file_new_for_path");
  GFileInfo *fi = g_file_query_info(f, G_FILE_ATTRIBUTE_STANDARD_TYPE,
                                    G_FILE_QUERY_INFO_NONE, NULL, NULL);
  QT_CHECK(fi && g_file_info_get_file_type(fi) == G_FILE_TYPE_DIRECTORY,
           "/tmp is reported as a directory");
  if (fi) g_object_unref(fi);
  g_object_unref(f);
  QT_CHECK(g_file_test("/tmp", G_FILE_TEST_IS_DIR), "g_file_test");
  QT_END();
}
