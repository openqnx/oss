/* nghttp2: drive a client and a server session against each other in memory,
 * exchanging a full HTTP/2 request and response over the framing layer. */
#include "common.h"
#include <stdlib.h>
#include <nghttp2/nghttp2.h>
struct peer { nghttp2_session *s; struct peer *other; int got_headers, got_data; };
static ssize_t send_cb(nghttp2_session *s, const uint8_t *data, size_t len,
                       int flags, void *user) {
  (void)s; (void)flags;
  struct peer *p = user;
  int rv = nghttp2_session_mem_recv(p->other->s, data, len);
  return rv < 0 ? NGHTTP2_ERR_CALLBACK_FAILURE : (ssize_t)len;
}
static int frame_recv_cb(nghttp2_session *s, const nghttp2_frame *f, void *user) {
  (void)s;
  struct peer *p = user;
  if (f->hd.type == NGHTTP2_HEADERS) ++p->got_headers;
  if (f->hd.type == NGHTTP2_DATA) ++p->got_data;
  return 0;
}
int main(void) {
  QT_BEGIN("nghttp2");
  nghttp2_info *info = nghttp2_version(0);
  printf("  info nghttp2 %s\n", info->version_str);
  QT_CHECK(info->version_num > 0, "nghttp2_version");
  nghttp2_session_callbacks *cb;
  nghttp2_session_callbacks_new(&cb);
  nghttp2_session_callbacks_set_send_callback(cb, send_cb);
  nghttp2_session_callbacks_set_on_frame_recv_callback(cb, frame_recv_cb);
  struct peer cli = {0}, srv = {0};
  cli.other = &srv; srv.other = &cli;
  QT_CHECK(nghttp2_session_client_new(&cli.s, cb, &cli) == 0, "client session");
  QT_CHECK(nghttp2_session_server_new(&srv.s, cb, &srv) == 0, "server session");
  nghttp2_session_callbacks_del(cb);
  nghttp2_submit_settings(cli.s, NGHTTP2_FLAG_NONE, NULL, 0);
  nghttp2_submit_settings(srv.s, NGHTTP2_FLAG_NONE, NULL, 0);
  nghttp2_nv nva[] = {
    { (uint8_t *)":method", (uint8_t *)"GET", 7, 3, NGHTTP2_NV_FLAG_NONE },
    { (uint8_t *)":scheme", (uint8_t *)"http", 7, 4, NGHTTP2_NV_FLAG_NONE },
    { (uint8_t *)":authority", (uint8_t *)"qnx", 10, 3, NGHTTP2_NV_FLAG_NONE },
    { (uint8_t *)":path", (uint8_t *)"/", 5, 1, NGHTTP2_NV_FLAG_NONE },
  };
  int32_t sid = nghttp2_submit_request(cli.s, NULL, nva, 4, NULL, NULL);
  QT_CHECK(sid > 0, "nghttp2_submit_request");
  QT_CHECK(nghttp2_session_send(cli.s) == 0, "client flush");
  QT_CHECK(srv.got_headers >= 1, "server received the HEADERS frame");
  nghttp2_nv rsp[] = {
    { (uint8_t *)":status", (uint8_t *)"200", 7, 3, NGHTTP2_NV_FLAG_NONE },
  };
  QT_CHECK(nghttp2_submit_response(srv.s, sid, rsp, 1, NULL) == 0, "submit_response");
  QT_CHECK(nghttp2_session_send(srv.s) == 0, "server flush");
  QT_CHECK(cli.got_headers >= 1, "client received the response HEADERS");
  nghttp2_session_del(cli.s);
  nghttp2_session_del(srv.s);
  QT_END();
}
