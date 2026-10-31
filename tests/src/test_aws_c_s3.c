/* aws-c-s3: the S3 client's meta-request machinery (connection pool, request
 * pipeline, SigV4 signing, body/finish callbacks, error-response handling) against a tiny
 * S3 stand-in written here, served over plain HTTP on 127.0.0.1: one object
 * that exists (200 + body) and one that does not (404 NoSuchKey). */
#include "common.h"
#include <unistd.h>
#include <pthread.h>
#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <aws/common/common.h>
#include <aws/common/condition_variable.h>
#include <aws/common/mutex.h>
#include <aws/common/uri.h>
#include <aws/io/io.h>
#include <aws/io/event_loop.h>
#include <aws/io/channel_bootstrap.h>
#include <aws/io/host_resolver.h>
#include <aws/http/request_response.h>
#include <aws/s3/s3.h>
#include <aws/s3/s3_client.h>
#include <aws/auth/credentials.h>
#include <aws/auth/signing_config.h>

static const char *OBJ = "The quick brown fox jumps over the lazy QNX dog.\n";
static int lfd;
static char seen_line[256];
static int seen_auth;
static void *serve_conn(void *arg) {
  int fd = (int)(intptr_t)arg;
  char buf[4096]; size_t have = 0;
  for (;;) {
    ssize_t r = read(fd, buf + have, sizeof buf - 1 - have);
    if (r <= 0) break;
    have += (size_t)r; buf[have] = 0;
    char *end;
    while ((end = strstr(buf, "\r\n\r\n")) != NULL) {
      size_t hl = (size_t)(end + 4 - buf);
      char line[256]; size_t ll = strcspn(buf, "\r");
      if (ll >= sizeof line) ll = sizeof line - 1;
      memcpy(line, buf, ll); line[ll] = 0;
      if (strstr(line, "hello.txt")) {
        snprintf(seen_line, sizeof seen_line, "%s", line);
        char save = buf[hl]; buf[hl] = 0;
        if (strstr(buf, "Authorization: AWS4-HMAC-SHA256 Credential=AKIDQNXTEST/") ||
            strstr(buf, "authorization: AWS4-HMAC-SHA256 Credential=AKIDQNXTEST/")) seen_auth = 1;
        buf[hl] = save;
      }
      char resp[1024]; int n;
      if (strncmp(line, "GET /qnx-bucket/hello.txt ", 26) == 0)
        n = snprintf(resp, sizeof resp, "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\nETag: \"0123abcd\"\r\n"
                     "Content-Length: %zu\r\n\r\n%s", strlen(OBJ), OBJ);
      else {
        const char *x = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n<Error><Code>NoSuchKey</Code>"
                        "<Message>The specified key does not exist.</Message></Error>";
        n = snprintf(resp, sizeof resp, "HTTP/1.1 404 Not Found\r\nContent-Type: application/xml\r\n"
                     "Content-Length: %zu\r\n\r\n%s", strlen(x), x);
      }
      if (write(fd, resp, (size_t)n) != n) break;
      memmove(buf, buf + hl, have - hl + 1); have -= hl;
    }
  }
  close(fd);
  return NULL;
}
static void *server(void *arg) {
  (void)arg;
  for (;;) {
    int fd = accept(lfd, NULL, NULL);
    if (fd < 0) return NULL;
    pthread_t t; pthread_create(&t, NULL, serve_conn, (void *)(intptr_t)fd); pthread_detach(t);
  }
}

static struct aws_mutex mtx = AWS_MUTEX_INIT;
static struct aws_condition_variable cv = AWS_CONDITION_VARIABLE_INIT;
static int done, cshut;
static bool pred_done(void *ud) { (void)ud; return done; }
static bool pred_shut(void *ud) { (void)ud; return cshut; }
static struct { int err, status; char body[256]; size_t blen; char errbody[512]; } res;
static int on_body(struct aws_s3_meta_request *m, const struct aws_byte_cursor *b, uint64_t start, void *ud) {
  (void)m; (void)start; (void)ud;
  if (res.blen + b->len < sizeof res.body) { memcpy(res.body + res.blen, b->ptr, b->len); res.blen += b->len; }
  return AWS_OP_SUCCESS;
}
static void on_finish(struct aws_s3_meta_request *m, const struct aws_s3_meta_request_result *r, void *ud) {
  (void)m; (void)ud;
  res.err = r->error_code; res.status = r->response_status;
  if (r->error_response_body)
    snprintf(res.errbody, sizeof res.errbody, "%.*s", (int)r->error_response_body->len, (const char *)r->error_response_body->buffer);
  aws_mutex_lock(&mtx); done = 1; aws_condition_variable_notify_all(&cv); aws_mutex_unlock(&mtx);
}
static void on_client_shutdown(void *ud) { (void)ud; aws_mutex_lock(&mtx); cshut = 1; aws_condition_variable_notify_all(&cv); aws_mutex_unlock(&mtx); }

static int get(struct aws_allocator *al, struct aws_s3_client *c, const struct aws_uri *ep, const char *hostport, const char *path) {
  memset(&res, 0, sizeof res);
  done = 0;
  struct aws_http_message *msg = aws_http_message_new_request(al);
  aws_http_message_set_request_method(msg, aws_byte_cursor_from_c_str("GET"));
  aws_http_message_set_request_path(msg, aws_byte_cursor_from_c_str(path));
  struct aws_http_header host = {aws_byte_cursor_from_c_str("Host"), aws_byte_cursor_from_c_str(hostport), 0};
  aws_http_message_add_header(msg, host);
  struct aws_s3_meta_request_options o = {0};
  o.type = AWS_S3_META_REQUEST_TYPE_DEFAULT;
  o.operation_name = aws_byte_cursor_from_c_str("GetObject");
  o.message = msg;
  o.endpoint = ep;
  o.body_callback = on_body;
  o.finish_callback = on_finish;
  struct aws_s3_meta_request *mr = aws_s3_client_make_meta_request(c, &o);
  int started = mr != NULL;
  if (!started) printf("  info make_meta_request: %s\n", aws_error_name(aws_last_error()));
  else {
    aws_mutex_lock(&mtx);
    aws_condition_variable_wait_for_pred(&cv, &mtx, 15LL * 1000000000LL, pred_done, NULL);
    aws_mutex_unlock(&mtx);
    aws_s3_meta_request_release(mr);
  }
  aws_http_message_release(msg);
  return started && done;
}

int main(void) {
  QT_BEGIN("aws-c-s3");
  alarm(90);
  struct aws_allocator *al = aws_default_allocator();
  aws_s3_library_init(al);

  lfd = socket(AF_INET, SOCK_STREAM, 0);
  struct sockaddr_in sa; memset(&sa, 0, sizeof sa);
  sa.sin_family = AF_INET; sa.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
  socklen_t sl = sizeof sa;
  int lok = bind(lfd, (struct sockaddr *)&sa, sizeof sa) == 0 && listen(lfd, 16) == 0 &&
            getsockname(lfd, (struct sockaddr *)&sa, &sl) == 0;
  QT_CHECK(lok, "S3 stand-in listening on 127.0.0.1");
  int port = ntohs(sa.sin_port);
  pthread_t st; pthread_create(&st, NULL, server, NULL); pthread_detach(st);
  char hostport[64], url[96];
  snprintf(hostport, sizeof hostport, "127.0.0.1:%d", port);
  snprintf(url, sizeof url, "http://127.0.0.1:%d", port);
  printf("  info endpoint %s\n", url);

  struct aws_event_loop_group_options eo = {0};
  eo.loop_count = 2;
  struct aws_event_loop_group *elg = aws_event_loop_group_new(al, &eo);
  struct aws_host_resolver_default_options ro = {0};
  ro.max_entries = 8; ro.el_group = elg;
  struct aws_host_resolver *hr = aws_host_resolver_new_default(al, &ro);
  struct aws_client_bootstrap_options cbo = {0};
  cbo.event_loop_group = elg; cbo.host_resolver = hr;
  struct aws_client_bootstrap *cb = aws_client_bootstrap_new(al, &cbo);

  /* The client insists on a signing config: SigV4 with static test keys
     (the stand-in server ignores the signature). */
  struct aws_credentials_provider_static_options sco;
  memset(&sco, 0, sizeof sco);
  sco.access_key_id = aws_byte_cursor_from_c_str("AKIDQNXTEST");
  sco.secret_access_key = aws_byte_cursor_from_c_str("qnx-test-secret");
  struct aws_credentials_provider *cp = aws_credentials_provider_new_static(al, &sco);
  struct aws_signing_config_aws sig;
  aws_s3_init_default_signing_config(&sig, aws_byte_cursor_from_c_str("us-east-1"), cp);
  struct aws_s3_client_config cfg = {0};
  cfg.signing_config = &sig;
  cfg.region = aws_byte_cursor_from_c_str("us-east-1");
  cfg.client_bootstrap = cb;
  cfg.tls_mode = AWS_MR_TLS_DISABLED;
  cfg.part_size = 8 * 1024 * 1024;
  cfg.throughput_target_gbps = 1.0;
  cfg.shutdown_callback = on_client_shutdown;
  struct aws_s3_client *c = aws_s3_client_new(al, &cfg);
  QT_CHECK(c != NULL, "S3 client created (TLS off, SigV4 with test keys)");
  if (!c) { printf("  info %s\n", aws_error_name(aws_last_error())); QT_END(); }

  struct aws_uri ep;
  struct aws_byte_cursor uc = aws_byte_cursor_from_c_str(url);
  aws_uri_init_parse(&ep, al, &uc);

  QT_CHECK(get(al, c, &ep, hostport, "/qnx-bucket/hello.txt"), "GetObject meta request finished");
  printf("  info status %d error %s, %zu body bytes\n", res.status, aws_error_name(res.err), res.blen);
  QT_CHECK(res.err == AWS_ERROR_SUCCESS && res.status == 200, "200 OK, no error");
  QT_CHECK(res.blen == strlen(OBJ) && memcmp(res.body, OBJ, res.blen) == 0, "object body delivered through body_callback");
  QT_CHECK(strncmp(seen_line, "GET /qnx-bucket/hello.txt HTTP/1.1", 34) == 0, "server saw the request line");
  QT_CHECK(seen_auth, "request arrived SigV4-signed (Authorization: AWS4-HMAC-SHA256 ...)");

  QT_CHECK(get(al, c, &ep, hostport, "/qnx-bucket/missing.txt"), "second meta request finished");
  printf("  info status %d error %s\n", res.status, aws_error_name(res.err));
  QT_CHECK(res.status == 404 && res.err != AWS_ERROR_SUCCESS, "404 reported as a failed meta request");
  QT_CHECK(strstr(res.errbody, "<Code>NoSuchKey</Code>") != NULL, "error response body handed to the caller");

  aws_uri_clean_up(&ep);
  aws_s3_client_release(c);
  aws_mutex_lock(&mtx);
  aws_condition_variable_wait_for_pred(&cv, &mtx, 10LL * 1000000000LL, pred_shut, NULL);
  aws_mutex_unlock(&mtx);
  QT_CHECK(cshut, "client shut down cleanly");
  aws_credentials_provider_release(cp);
  aws_client_bootstrap_release(cb);
  aws_host_resolver_release(hr);
  aws_event_loop_group_release(elg);
  aws_s3_library_clean_up();
  close(lfd);
  QT_END();
}
