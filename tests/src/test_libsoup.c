/* libsoup: run a real HTTP request against a SoupServer listening on the
 * loopback interface inside this same process. */
#include "common.h"
#include <libsoup/soup.h>
static void handler(SoupServer *server, SoupServerMessage *msg, const char *path,
                    GHashTable *query, gpointer user) {
  (void)server; (void)query; (void)user;
  if (strcmp(path, "/ports") == 0) {
    static const char body[] = "zlib,zstd";
    soup_server_message_set_status(msg, SOUP_STATUS_OK, NULL);
    soup_server_message_set_response(msg, "text/plain", SOUP_MEMORY_STATIC,
                                     body, sizeof body - 1);
  } else {
    soup_server_message_set_status(msg, SOUP_STATUS_NOT_FOUND, NULL);
  }
}
/* The server needs a main context that keeps running while the client call
 * below blocks; sharing the default context would deadlock the two. */
static GMainContext *server_ctx;
static GMainLoop *server_loop;
static SoupServer *server;
static GMutex ready_mutex;
static GCond ready_cond;
static gboolean ready;
static char *server_base;
static gpointer server_thread(gpointer data) {
  (void)data;
  g_main_context_push_thread_default(server_ctx);
  server = soup_server_new("server-header", "qnx-port-test", NULL);
  soup_server_add_handler(server, "/", handler, NULL, NULL);
  GError *e = NULL;
  if (soup_server_listen_local(server, 0, 0, &e)) {
    GSList *uris = soup_server_get_uris(server);
    if (uris) {
      server_base = g_uri_to_string((GUri *)uris->data);
      g_slist_free_full(uris, (GDestroyNotify)g_uri_unref);
    }
  } else if (e) {
    g_printerr("  info %s\n", e->message);
    g_clear_error(&e);
  }
  g_mutex_lock(&ready_mutex);
  ready = TRUE;
  g_cond_signal(&ready_cond);
  g_mutex_unlock(&ready_mutex);
  g_main_loop_run(server_loop);
  g_main_context_pop_thread_default(server_ctx);
  return NULL;
}
int main(void) {
  QT_BEGIN("libsoup");
  printf("  info libsoup %u.%u.%u\n", soup_get_major_version(),
         soup_get_minor_version(), soup_get_micro_version());
  GError *err = NULL;
  g_mutex_init(&ready_mutex);
  g_cond_init(&ready_cond);
  server_ctx = g_main_context_new();
  server_loop = g_main_loop_new(server_ctx, FALSE);
  GThread *th = g_thread_new("soup-server", server_thread, NULL);
  g_mutex_lock(&ready_mutex);
  while (!ready) g_cond_wait(&ready_cond, &ready_mutex);
  g_mutex_unlock(&ready_mutex);
  QT_CHECK(server != NULL, "soup_server_new");
  QT_CHECK(server_base != NULL, "soup_server_listen_local and a listening URI");
  if (!server_base) QT_END();
  char *base = server_base;
  printf("  info listening on %s\n", base);

  char *url = g_strconcat(base, "ports", NULL);
  SoupSession *session = soup_session_new();
  SoupMessage *msg = soup_message_new("GET", url);
  QT_CHECK(msg != NULL, "soup_message_new");
  GBytes *body = soup_session_send_and_read(session, msg, NULL, &err);
  QT_CHECK(body != NULL, "soup_session_send_and_read");
  if (err) printf("  info %s\n", err->message);
  QT_CHECK(soup_message_get_status(msg) == SOUP_STATUS_OK, "HTTP 200");
  if (body) {
    gsize n = 0;
    const char *data = g_bytes_get_data(body, &n);
    QT_CHECK(n == 9 && memcmp(data, "zlib,zstd", 9) == 0, "response body matches");
    g_bytes_unref(body);
  }
  g_object_unref(msg);

  char *bad = g_strconcat(base, "missing", NULL);
  msg = soup_message_new("GET", bad);
  GBytes *b2 = soup_session_send_and_read(session, msg, NULL, NULL);
  QT_CHECK(soup_message_get_status(msg) == SOUP_STATUS_NOT_FOUND, "HTTP 404 for an unknown path");
  if (b2) g_bytes_unref(b2);
  g_object_unref(msg);

  g_free(url); g_free(bad);
  g_object_unref(session);
  g_main_loop_quit(server_loop);
  g_thread_join(th);
  soup_server_disconnect(server);
  g_object_unref(server);
  g_main_loop_unref(server_loop);
  g_main_context_unref(server_ctx);
  g_free(base);
  QT_END();
}
