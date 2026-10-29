/* civetweb: embedded server on loopback, fetched with its own client API. */
#include "common.h"
#include <civetweb.h>
static int h(struct mg_connection *c, void *u) {
  mg_printf(c, "HTTP/1.1 200 OK\r\nContent-Length: 5\r\nConnection: close\r\n\r\nhello"); return 200;
}
int main(void) {
  QT_BEGIN("civetweb");
  QT_CHECK(mg_init_library(MG_FEATURES_SSL) & MG_FEATURES_SSL, "TLS support");
  const char *opts[] = {"listening_ports", "127.0.0.1:18081", "num_threads", "2", NULL};
  struct mg_context *ctx = mg_start(NULL, NULL, opts);
  QT_CHECK(ctx != NULL, "server starts");
  mg_set_request_handler(ctx, "/t", h, NULL);
  char err[128]; struct mg_connection *c = mg_download("127.0.0.1", 18081, 0, err, sizeof err, "GET /t HTTP/1.0\r\n\r\n");
  QT_CHECK(c != NULL, "client request");
  char body[16] = {0}; if (c) { mg_read(c, body, sizeof body - 1); mg_close_connection(c); }
  QT_CHECK(!strcmp(body, "hello"), "response body");
  mg_stop(ctx); mg_exit_library();
  QT_END();
}
