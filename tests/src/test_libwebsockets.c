#include "common.h"
#include <libwebsockets.h>
/* Brings up a real lws context (no listen socket) and tears it down. */
static int cb(struct lws *w, enum lws_callback_reasons r, void *u, void *in, size_t l) {
  (void)w; (void)r; (void)u; (void)in; (void)l; return 0;
}
int main(void) {
  QT_BEGIN("libwebsockets");
  static struct lws_protocols protocols[] = {
    { "qnx-test", cb, 0, 0, 0, NULL, 0 },
    LWS_PROTOCOL_LIST_TERM
  };
  struct lws_context_creation_info info;
  memset(&info, 0, sizeof(info));
  info.port = CONTEXT_PORT_NO_LISTEN;
  info.protocols = protocols;
  info.gid = -1; info.uid = -1;
  struct lws_context *ctx = lws_create_context(&info);
  QT_CHECK(ctx != NULL, "lws_create_context");
  if (ctx) {
    /* One non-blocking service pass proves the event loop runs on QNX. */
    int rc = lws_service(ctx, 0);
    QT_CHECK(rc >= 0, "lws_service pass");
    lws_context_destroy(ctx);
    QT_CHECK(1, "lws_context_destroy");
  }
  printf("  info %s\n", lws_get_library_version());
  QT_END();
}
