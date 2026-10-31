/* aws-c-http: an HTTP/1.1 server and client from the library talking over
 * 127.0.0.1 -- the request's method, path, header and body reach the server
 * handler, the response's status, header and body reach the client; plus
 * header-collection and status-text helpers. */
#include "common.h"
#include <unistd.h>
#include <stdio.h>
#include <aws/common/common.h>
#include <aws/common/condition_variable.h>
#include <aws/common/mutex.h>
#include <aws/io/io.h>
#include <aws/io/event_loop.h>
#include <aws/io/channel_bootstrap.h>
#include <aws/io/host_resolver.h>
#include <aws/io/socket.h>
#include <aws/io/stream.h>
#include <aws/http/http.h>
#include <aws/http/server.h>
#include <aws/http/connection.h>
#include <aws/http/request_response.h>
#include <aws/http/status_code.h>

static struct aws_allocator *al;
static struct aws_mutex mtx = AWS_MUTEX_INIT;
static struct aws_condition_variable cv = AWS_CONDITION_VARIABLE_INIT;
static int flags, want;
enum { F_SETUP = 1, F_SRV_REQ = 2, F_RESP = 4, F_SHUT = 8, F_SRV_GONE = 16, F_SETUP_ERR = 32 };
static void set_flag(int f) { aws_mutex_lock(&mtx); flags |= f; aws_condition_variable_notify_all(&cv); aws_mutex_unlock(&mtx); }
static bool pred(void *ud) { (void)ud; return (flags & want) == want || (flags & F_SETUP_ERR); }
static int wait_flags(int f) {
  aws_mutex_lock(&mtx); want = f;
  aws_condition_variable_wait_for_pred(&cv, &mtx, 10LL * 1000000000LL, pred, NULL);
  int ok = (flags & f) == f; aws_mutex_unlock(&mtx); return ok;
}

/* ---- server side ---- */
struct srv_req { char method[16], path[64], hdr[64], body[64]; size_t blen; struct aws_http_message *resp;
                 struct aws_input_stream *body_stream; char resp_body[96]; };
static struct srv_req sreq;
static int srv_headers(struct aws_http_stream *s, enum aws_http_header_block b, const struct aws_http_header *h, size_t n, void *ud) {
  (void)s; (void)b; (void)ud;
  for (size_t i = 0; i < n; ++i)
    if (aws_byte_cursor_eq_c_str_ignore_case(&h[i].name, "x-client"))
      snprintf(sreq.hdr, sizeof sreq.hdr, "%.*s", (int)h[i].value.len, (const char *)h[i].value.ptr);
  return AWS_OP_SUCCESS;
}
static int srv_body(struct aws_http_stream *s, const struct aws_byte_cursor *d, void *ud) {
  (void)s; (void)ud;
  if (sreq.blen + d->len < sizeof sreq.body) { memcpy(sreq.body + sreq.blen, d->ptr, d->len); sreq.blen += d->len; }
  return AWS_OP_SUCCESS;
}
static int srv_done(struct aws_http_stream *s, void *ud) {
  (void)ud;
  struct aws_byte_cursor m, u;
  aws_http_stream_get_incoming_request_method(s, &m);
  aws_http_stream_get_incoming_request_uri(s, &u);
  snprintf(sreq.method, sizeof sreq.method, "%.*s", (int)m.len, (const char *)m.ptr);
  snprintf(sreq.path, sizeof sreq.path, "%.*s", (int)u.len, (const char *)u.ptr);
  snprintf(sreq.resp_body, sizeof sreq.resp_body, "you sent %zu bytes to %s", sreq.blen, sreq.path);
  set_flag(F_SRV_REQ);
  struct aws_http_message *r = aws_http_message_new_response(al);
  aws_http_message_set_response_status(r, 201);
  char cl[16]; snprintf(cl, sizeof cl, "%zu", strlen(sreq.resp_body));
  struct aws_http_header h1 = {aws_byte_cursor_from_c_str("Content-Length"), aws_byte_cursor_from_c_str(cl), 0};
  struct aws_http_header h2 = {aws_byte_cursor_from_c_str("X-Server"), aws_byte_cursor_from_c_str("aws-c-http on QNX"), 0};
  aws_http_message_add_header(r, h1);
  aws_http_message_add_header(r, h2);
  struct aws_byte_cursor bc = aws_byte_cursor_from_c_str(sreq.resp_body);
  sreq.body_stream = aws_input_stream_new_from_cursor(al, &bc);
  aws_http_message_set_body_stream(r, sreq.body_stream);
  sreq.resp = r;
  return aws_http_stream_send_response(s, r);
}
static void srv_complete(struct aws_http_stream *s, int err, void *ud) {
  (void)err; (void)ud;
  aws_http_stream_release(s);
  if (sreq.resp) { aws_http_message_release(sreq.resp); sreq.resp = NULL; }
  if (sreq.body_stream) { aws_input_stream_release(sreq.body_stream); sreq.body_stream = NULL; }
}
static struct aws_http_stream *srv_incoming_request(struct aws_http_connection *c, void *ud) {
  (void)ud;
  struct aws_http_request_handler_options o = AWS_HTTP_REQUEST_HANDLER_OPTIONS_INIT;
  o.server_connection = c;
  o.on_request_headers = srv_headers;
  o.on_request_body = srv_body;
  o.on_request_done = srv_done;
  o.on_complete = srv_complete;
  return aws_http_stream_new_server_request_handler(&o);
}
static void srv_conn_shutdown(struct aws_http_connection *c, int err, void *ud) { (void)err; (void)ud; aws_http_connection_release(c); }
static void srv_incoming_conn(struct aws_http_server *srv, struct aws_http_connection *c, int err, void *ud) {
  (void)srv; (void)ud;
  if (err || !c) { printf("  info server connection error %d\n", err); return; }
  struct aws_http_server_connection_options o = AWS_HTTP_SERVER_CONNECTION_OPTIONS_INIT;
  o.on_incoming_request = srv_incoming_request;
  o.on_shutdown = srv_conn_shutdown;
  aws_http_connection_configure_server(c, &o);
}
static void srv_destroyed(void *ud) { (void)ud; set_flag(F_SRV_GONE); }

/* ---- client side ---- */
static struct aws_http_connection *cconn;
static int resp_status; static char resp_hdr[64], resp_body[128]; static size_t resp_len;
static int cli_headers(struct aws_http_stream *s, enum aws_http_header_block b, const struct aws_http_header *h, size_t n, void *ud) {
  (void)b; (void)ud;
  aws_http_stream_get_incoming_response_status(s, &resp_status);
  for (size_t i = 0; i < n; ++i)
    if (aws_byte_cursor_eq_c_str_ignore_case(&h[i].name, "x-server"))
      snprintf(resp_hdr, sizeof resp_hdr, "%.*s", (int)h[i].value.len, (const char *)h[i].value.ptr);
  return AWS_OP_SUCCESS;
}
static int cli_body(struct aws_http_stream *s, const struct aws_byte_cursor *d, void *ud) {
  (void)s; (void)ud;
  if (resp_len + d->len < sizeof resp_body) { memcpy(resp_body + resp_len, d->ptr, d->len); resp_len += d->len; }
  return AWS_OP_SUCCESS;
}
static struct aws_http_message *creq;
static struct aws_input_stream *creq_body;
static void cli_complete(struct aws_http_stream *s, int err, void *ud) {
  (void)ud;
  if (err) printf("  info client stream error %d (%s)\n", err, aws_error_name(err));
  aws_http_stream_release(s);
  set_flag(F_RESP);
}
static void cli_setup(struct aws_http_connection *c, int err, void *ud) {
  (void)ud;
  if (err) { printf("  info client setup error %d (%s)\n", err, aws_error_name(err)); set_flag(F_SETUP_ERR); return; }
  cconn = c;
  set_flag(F_SETUP);
  creq = aws_http_message_new_request(al);
  aws_http_message_set_request_method(creq, aws_byte_cursor_from_c_str("PUT"));
  aws_http_message_set_request_path(creq, aws_byte_cursor_from_c_str("/objects/42?v=1"));
  static const char body[] = "payload-from-client";
  struct aws_http_header hs[] = {
    {aws_byte_cursor_from_c_str("Host"), aws_byte_cursor_from_c_str("127.0.0.1"), 0},
    {aws_byte_cursor_from_c_str("X-Client"), aws_byte_cursor_from_c_str("qnx-test"), 0},
    {aws_byte_cursor_from_c_str("Content-Length"), aws_byte_cursor_from_c_str("19"), 0}};
  for (int i = 0; i < 3; ++i) aws_http_message_add_header(creq, hs[i]);
  struct aws_byte_cursor bc = aws_byte_cursor_from_c_str(body);
  creq_body = aws_input_stream_new_from_cursor(al, &bc);
  aws_http_message_set_body_stream(creq, creq_body);
  struct aws_http_make_request_options ro = {0};
  ro.self_size = sizeof ro;
  ro.request = creq;
  ro.on_response_headers = cli_headers;
  ro.on_response_body = cli_body;
  ro.on_complete = cli_complete;
  struct aws_http_stream *st = aws_http_connection_make_request(c, &ro);
  if (!st || aws_http_stream_activate(st)) printf("  info make_request failed: %s\n", aws_error_name(aws_last_error()));
}
static void cli_shutdown(struct aws_http_connection *c, int err, void *ud) { (void)c; (void)err; (void)ud; set_flag(F_SHUT); }

int main(void) {
  QT_BEGIN("aws-c-http");
  alarm(60);
  al = aws_default_allocator();
  aws_http_library_init(al);

  QT_CHECK(strcmp(aws_http_status_text(404), "Not Found") == 0, "status text for 404");
  struct aws_http_headers *hh = aws_http_headers_new(al);
  aws_http_headers_add(hh, aws_byte_cursor_from_c_str("Accept"), aws_byte_cursor_from_c_str("a"));
  aws_http_headers_add(hh, aws_byte_cursor_from_c_str("accept"), aws_byte_cursor_from_c_str("b"));
  struct aws_byte_cursor v;
  QT_CHECK(aws_http_headers_count(hh) == 2 && aws_http_headers_get(hh, aws_byte_cursor_from_c_str("ACCEPT"), &v) == AWS_OP_SUCCESS &&
           aws_byte_cursor_eq_c_str(&v, "a"), "header lookup is case-insensitive, first wins");
  aws_http_headers_release(hh);

  struct aws_event_loop_group_options eo = {0};
  eo.loop_count = 1;
  struct aws_event_loop_group *elg = aws_event_loop_group_new(al, &eo);
  struct aws_host_resolver_default_options ro = {0};
  ro.max_entries = 8; ro.el_group = elg;
  struct aws_host_resolver *res = aws_host_resolver_new_default(al, &ro);
  struct aws_client_bootstrap_options cbo = {0};
  cbo.event_loop_group = elg; cbo.host_resolver = res;
  struct aws_client_bootstrap *cb = aws_client_bootstrap_new(al, &cbo);
  struct aws_server_bootstrap *sb = aws_server_bootstrap_new(al, elg);
  QT_CHECK(elg && res && cb && sb, "event loop, resolver and bootstraps");

  struct aws_socket_options so = {0};
  so.type = AWS_SOCKET_STREAM; so.domain = AWS_SOCKET_IPV4; so.connect_timeout_ms = 5000;
  struct aws_socket_endpoint ep = {0};
  strcpy(ep.address, "127.0.0.1");
  struct aws_http_server_options sopt = AWS_HTTP_SERVER_OPTIONS_INIT;
  sopt.allocator = al; sopt.bootstrap = sb; sopt.endpoint = &ep; sopt.socket_options = &so;
  sopt.on_incoming_connection = srv_incoming_conn; sopt.on_destroy_complete = srv_destroyed;
  struct aws_http_server *srv = aws_http_server_new(&sopt);
  QT_CHECK(srv != NULL, "HTTP server listening");
  if (!srv) { printf("  info %s\n", aws_error_name(aws_last_error())); QT_END(); }
  uint32_t port = aws_http_server_get_listener_endpoint(srv)->port;
  printf("  info server on 127.0.0.1:%u\n", (unsigned)port);
  QT_CHECK(port > 0, "ephemeral port assigned");

  struct aws_http_client_connection_options copt = AWS_HTTP_CLIENT_CONNECTION_OPTIONS_INIT;
  copt.allocator = al; copt.bootstrap = cb; copt.host_name = aws_byte_cursor_from_c_str("127.0.0.1");
  copt.port = port; copt.socket_options = &so; copt.on_setup = cli_setup; copt.on_shutdown = cli_shutdown;
  QT_CHECK(aws_http_client_connect(&copt) == AWS_OP_SUCCESS, "client connect started");
  QT_CHECK(wait_flags(F_SETUP), "client connection set up");
  QT_CHECK(wait_flags(F_SRV_REQ | F_RESP), "request served and response received");
  QT_CHECK(strcmp(sreq.method, "PUT") == 0 && strcmp(sreq.path, "/objects/42?v=1") == 0, "server saw PUT /objects/42?v=1");
  QT_CHECK(strcmp(sreq.hdr, "qnx-test") == 0, "server saw the X-Client header");
  QT_CHECK(sreq.blen == 19 && memcmp(sreq.body, "payload-from-client", 19) == 0, "server received the request body");
  printf("  info response %d \"%.*s\"\n", resp_status, (int)resp_len, resp_body);
  QT_CHECK(resp_status == 201, "client got status 201");
  QT_CHECK(strcmp(resp_hdr, "aws-c-http on QNX") == 0, "client got the X-Server header");
  QT_CHECK(resp_len == strlen("you sent 19 bytes to /objects/42?v=1") &&
           memcmp(resp_body, "you sent 19 bytes to /objects/42?v=1", resp_len) == 0, "client got the response body");

  if (cconn) { aws_http_connection_close(cconn); wait_flags(F_SHUT); aws_http_connection_release(cconn); }
  QT_CHECK(flags & F_SHUT, "client connection shut down");
  aws_http_server_release(srv);
  QT_CHECK(wait_flags(F_SRV_GONE), "server destroyed");
  if (creq) aws_http_message_release(creq);
  if (creq_body) aws_input_stream_release(creq_body);
  aws_client_bootstrap_release(cb);
  aws_server_bootstrap_release(sb);
  aws_host_resolver_release(res);
  aws_event_loop_group_release(elg);
  aws_http_library_clean_up();
  QT_END();
}
