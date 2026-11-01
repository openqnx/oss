/* neon: HTTP/WebDAV client requests against a tiny HTTP/1.1 server the test
 * runs in a thread on 127.0.0.1. Checks: a GET with a body reader (status,
 * reason, body), persistent connection reuse, a gzip-encoded response
 * decoded by ne_decompress_reader (zlib), PUT then GET of the same bytes,
 * Basic authentication after a 401 challenge (credentials callback called
 * once), 404 reported as NE_ERROR with the status line, DELETE and MKCOL,
 * a PROPFIND (207 Multi-Status) parsed into two resources with their
 * properties (expat), and the URI, base64 and date helpers. */
#include "common.h"
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <strings.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <zlib.h>
#include <neon/ne_session.h>
#include <neon/ne_request.h>
#include <neon/ne_basic.h>
#include <neon/ne_auth.h>
#include <neon/ne_compress.h>
#include <neon/ne_props.h>
#include <neon/ne_uri.h>
#include <neon/ne_string.h>
#include <neon/ne_dates.h>
#include <neon/ne_socket.h>
#include <neon/ne_utils.h>

/* ---- the server ---------------------------------------------------------- */
static int lsock;
static int connections;           /* accepted TCP connections */
static char stored[4096]; static int stored_len = -1;
static const char *TEXT = "neon on QNX: the quick brown fox jumps over the lazy dog. ";
static unsigned char gz[1024]; static int gz_len;

static void reply(int fd, const char *status, const char *extra, const void *body, int len) {
  char h[512];
  int n = snprintf(h, sizeof h, "HTTP/1.1 %s\r\nContent-Length: %d\r\n%s\r\n", status, len, extra ? extra : "");
  (void)!write(fd, h, n);
  if (len > 0) (void)!write(fd, body, len);
}
static int has(const char *hdrs, const char *needle) { return strcasestr(hdrs, needle) != NULL; }

static void serve(int fd) {
  static char buf[16384];
  int have = 0;
  for (;;) {
    char *end;
    while (!(end = (have ? strstr(buf, "\r\n\r\n") : NULL))) {
      int r = (int)read(fd, buf + have, sizeof buf - 1 - have);
      if (r <= 0) return;
      have += r; buf[have] = 0;
    }
    int hlen = (int)(end + 4 - buf);
    char method[16] = "", path[256] = "";
    sscanf(buf, "%15s %255s", method, path);
    int clen = 0; const char *cl = strcasestr(buf, "\r\nContent-Length:");
    if (cl && cl < end) clen = atoi(cl + 17);
    while (have < hlen + clen) { int r = (int)read(fd, buf + have, sizeof buf - 1 - have); if (r <= 0) return; have += r; }
    buf[have] = 0;
    char hdrs[4096]; int hl = hlen < (int)sizeof hdrs - 1 ? hlen : (int)sizeof hdrs - 1;
    memcpy(hdrs, buf, hl); hdrs[hl] = 0;
    const char *body = buf + hlen;

    if (!strcmp(method, "GET") && !strcmp(path, "/hello")) {
      reply(fd, "200 OK", "Content-Type: text/plain\r\n", "Hello neon", 10);
    } else if (!strcmp(method, "GET") && !strcmp(path, "/gz")) {
      if (has(hdrs, "Accept-Encoding: gzip")) reply(fd, "200 OK", "Content-Encoding: gzip\r\n", gz, gz_len);
      else reply(fd, "200 OK", NULL, TEXT, (int)strlen(TEXT));
    } else if (!strcmp(method, "PUT") && !strcmp(path, "/up")) {
      stored_len = clen < (int)sizeof stored ? clen : (int)sizeof stored;
      memcpy(stored, body, stored_len);
      reply(fd, "201 Created", NULL, "", 0);
    } else if (!strcmp(method, "GET") && !strcmp(path, "/up")) {
      if (stored_len < 0) reply(fd, "404 Not Found", NULL, "", 0);
      else reply(fd, "200 OK", NULL, stored, stored_len);
    } else if (!strcmp(method, "DELETE") && !strcmp(path, "/up")) {
      stored_len = -1; reply(fd, "204 No Content", NULL, "", 0);
    } else if (!strcmp(method, "MKCOL") && !strcmp(path, "/dav/new/")) {
      reply(fd, "201 Created", NULL, "", 0);
    } else if (!strcmp(method, "GET") && !strcmp(path, "/secret")) {
      /* "qnx:port" in base64 */
      if (has(hdrs, "Authorization: Basic cW54OnBvcnQ=")) reply(fd, "200 OK", NULL, "top secret", 10);
      else reply(fd, "401 Unauthorized", "WWW-Authenticate: Basic realm=\"qnxtest\"\r\n", "", 0);
    } else if (!strcmp(method, "PROPFIND") && !strcmp(path, "/dav/")) {
      static const char ms[] =
        "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n"
        "<D:multistatus xmlns:D=\"DAV:\">\n"
        "<D:response><D:href>/dav/</D:href><D:propstat><D:prop>"
        "<D:displayname>dav root</D:displayname><D:getcontentlength>0</D:getcontentlength>"
        "</D:prop><D:status>HTTP/1.1 200 OK</D:status></D:propstat></D:response>\n"
        "<D:response><D:href>/dav/file.txt</D:href><D:propstat><D:prop>"
        "<D:displayname>file.txt</D:displayname><D:getcontentlength>1234</D:getcontentlength>"
        "</D:prop><D:status>HTTP/1.1 200 OK</D:status></D:propstat></D:response>\n"
        "</D:multistatus>\n";
      reply(fd, "207 Multi-Status", "Content-Type: application/xml; charset=utf-8\r\n", ms, (int)strlen(ms));
    } else {
      reply(fd, "404 Not Found", "Content-Type: text/plain\r\n", "no such thing", 13);
    }
    memmove(buf, buf + hlen + clen, have - hlen - clen + 1);
    have -= hlen + clen;
  }
}
static void *server(void *arg) {
  (void)arg;
  for (;;) {
    int fd = accept(lsock, NULL, NULL);
    if (fd < 0) return NULL;
    __sync_fetch_and_add(&connections, 1);
    serve(fd);
    close(fd);
  }
}

/* ---- client helpers ------------------------------------------------------ */
struct sink { char buf[8192]; size_t len; };
static int collect(void *ud, const char *data, size_t len) {
  struct sink *s = ud;
  if (s->len + len >= sizeof s->buf) return -1;
  memcpy(s->buf + s->len, data, len); s->len += len; s->buf[s->len] = 0;
  return 0;
}
static int auth_calls;
static int creds(void *ud, const char *realm, int attempt, char *user, char *pass) {
  (void)ud; auth_calls++;
  if (attempt > 0 || strcmp(realm, "qnxtest") != 0) return -1;
  strcpy(user, "qnx"); strcpy(pass, "port");
  return 0;
}
struct props { int count; int file_len_ok; int root_name_ok; };
static const ne_propname PROPS[] = { { "DAV:", "displayname" }, { "DAV:", "getcontentlength" }, { NULL, NULL } };
static void on_props(void *ud, const ne_uri *uri, const ne_prop_result_set *set) {
  struct props *p = ud;
  p->count++;
  const char *name = ne_propset_value(set, &PROPS[0]), *len = ne_propset_value(set, &PROPS[1]);
  if (!strcmp(uri->path, "/dav/file.txt") && len && !strcmp(len, "1234")) p->file_len_ok = 1;
  if (!strcmp(uri->path, "/dav/") && name && !strcmp(name, "dav root")) p->root_name_ok = 1;
}

int main(void) {
  QT_BEGIN("neon");
  /* gzip body for /gz */
  z_stream zs; memset(&zs, 0, sizeof zs);
  deflateInit2(&zs, 9, Z_DEFLATED, 16 + MAX_WBITS, 8, Z_DEFAULT_STRATEGY);
  zs.next_in = (unsigned char *)TEXT; zs.avail_in = (unsigned)strlen(TEXT);
  zs.next_out = gz; zs.avail_out = sizeof gz;
  deflate(&zs, Z_FINISH); gz_len = (int)zs.total_out; deflateEnd(&zs);

  lsock = socket(AF_INET, SOCK_STREAM, 0);
  int one = 1; setsockopt(lsock, SOL_SOCKET, SO_REUSEADDR, &one, sizeof one);
  struct sockaddr_in a; memset(&a, 0, sizeof a);
  a.sin_family = AF_INET; a.sin_addr.s_addr = htonl(INADDR_LOOPBACK); a.sin_port = 0;
  socklen_t al = sizeof a;
  int ok = bind(lsock, (struct sockaddr *)&a, sizeof a) == 0 && listen(lsock, 8) == 0 &&
           getsockname(lsock, (struct sockaddr *)&a, &al) == 0;
  QT_CHECK(ok, "test server listening on 127.0.0.1");
  if (!ok) QT_END();
  unsigned port = ntohs(a.sin_port);
  pthread_t th; pthread_create(&th, NULL, server, NULL);

  QT_CHECK(ne_sock_init() == 0, "ne_sock_init");
  printf("  info %s\n", ne_version_string());
  QT_CHECK(ne_has_support(NE_FEATURE_SSL) && ne_has_support(NE_FEATURE_ZLIB), "built with SSL and zlib support");
  ne_session *s = ne_session_create("http", "127.0.0.1", port);
  ne_set_read_timeout(s, 10);
  ne_set_useragent(s, "qnx-port-test/1.0");

  /* GET with a body reader, twice: one connection */
  for (int i = 0; i < 2; i++) {
    struct sink k = { .len = 0 };
    ne_request *r = ne_request_create(s, "GET", "/hello");
    ne_add_response_body_reader(r, ne_accept_2xx, collect, &k);
    int rc = ne_request_dispatch(r);
    const ne_status *st = ne_get_status(r);
    if (i == 0) {
      QT_CHECK(rc == NE_OK && st->code == 200 && !strcmp(st->reason_phrase, "OK"), "GET /hello: 200 OK");
      QT_CHECK(!strcmp(k.buf, "Hello neon"), "GET /hello: body");
      const char *ct = ne_get_response_header(r, "Content-Type");
      QT_CHECK(ct && !strcmp(ct, "text/plain"), "response header Content-Type");
    }
    ne_request_destroy(r);
  }
  QT_CHECK(connections == 1, "two requests on one persistent connection");

  /* gzip */
  {
    struct sink k = { .len = 0 };
    ne_request *r = ne_request_create(s, "GET", "/gz");
    ne_decompress *dc = ne_decompress_reader(r, ne_accept_2xx, collect, &k);
    int rc = ne_request_dispatch(r);
    ne_decompress_destroy(dc);
    QT_CHECK(rc == NE_OK && k.len == strlen(TEXT) && !strcmp(k.buf, TEXT), "gzip response decoded (zlib)");
    ne_request_destroy(r);
  }

  /* PUT, GET back, DELETE */
  {
    static const char data[] = "line 1\nline 2\n\x01\x02 binary too";
    QT_CHECK(ne_putbuf(s, "/up", data, sizeof data - 1) == NE_OK, "ne_putbuf: 201 Created");
    QT_CHECK(stored_len == (int)sizeof data - 1 && !memcmp(stored, data, stored_len), "server received the exact bytes");
    struct sink k = { .len = 0 };
    ne_request *r = ne_request_create(s, "GET", "/up");
    ne_add_response_body_reader(r, ne_accept_2xx, collect, &k);
    QT_CHECK(ne_request_dispatch(r) == NE_OK && k.len == sizeof data - 1 && !memcmp(k.buf, data, k.len), "GET returns what was PUT");
    ne_request_destroy(r);
    QT_CHECK(ne_delete(s, "/up") == NE_OK && stored_len == -1, "ne_delete");
    QT_CHECK(ne_mkcol(s, "/dav/new/") == NE_OK, "ne_mkcol: 201 Created");
  }

  /* 404 */
  {
    ne_request *r = ne_request_create(s, "GET", "/nothing-here");
    int rc = ne_request_dispatch(r);
    QT_CHECK(rc == NE_OK && ne_get_status(r)->code == 404, "dispatch of a missing resource: status 404");
    ne_request_destroy(r);
    QT_CHECK(ne_delete(s, "/nothing-here") == NE_ERROR && strstr(ne_get_error(s), "404") != NULL, "ne_delete on 404: NE_ERROR naming the status");
    printf("  info error: %s\n", ne_get_error(s));
  }

  /* Basic auth */
  {
    ne_set_server_auth(s, creds, NULL);
    struct sink k = { .len = 0 };
    ne_request *r = ne_request_create(s, "GET", "/secret");
    ne_add_response_body_reader(r, ne_accept_2xx, collect, &k);
    int rc = ne_request_dispatch(r);
    QT_CHECK(rc == NE_OK && ne_get_status(r)->code == 200 && !strcmp(k.buf, "top secret"), "Basic auth after a 401 challenge");
    QT_CHECK(auth_calls == 1, "credentials asked for once, for realm qnxtest");
    ne_request_destroy(r);
  }

  /* PROPFIND */
  {
    struct props p = { 0, 0, 0 };
    int rc = ne_simple_propfind(s, "/dav/", NE_DEPTH_ONE, PROPS, on_props, &p);
    QT_CHECK(rc == NE_OK && p.count == 2, "PROPFIND: 207 with two resources");
    QT_CHECK(p.file_len_ok && p.root_name_ok, "PROPFIND: getcontentlength and displayname values");
  }
  ne_session_destroy(s);

  /* helpers */
  ne_uri u;
  QT_CHECK(ne_uri_parse("https://user@example.org:8443/a/b%20c?q=1#f", &u) == 0 && !strcmp(u.scheme, "https") &&
           u.port == 8443 && !strcmp(u.host, "example.org") && !strcmp(u.path, "/a/b%20c") && !strcmp(u.query, "q=1"),
           "ne_uri_parse");
  char *unesc = ne_path_unescape(u.path);
  QT_CHECK(unesc && !strcmp(unesc, "/a/b c"), "ne_path_unescape");
  free(unesc); ne_uri_free(&u);
  char *b64 = ne_base64((const unsigned char *)"qnx:port", 8);
  QT_CHECK(b64 && !strcmp(b64, "cW54OnBvcnQ="), "ne_base64");
  free(b64);
  QT_CHECK(ne_rfc1123_parse("Sun, 06 Nov 1994 08:49:37 GMT") == 784111777, "ne_rfc1123_parse");
  char *d = ne_rfc1123_date(784111777);
  QT_CHECK(d && !strcmp(d, "Sun, 06 Nov 1994 08:49:37 GMT"), "ne_rfc1123_date");
  free(d);

  shutdown(lsock, SHUT_RDWR); close(lsock);
  ne_sock_exit();
  QT_END();
}
