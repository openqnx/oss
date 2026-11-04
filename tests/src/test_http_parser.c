/* http-parser: parse real HTTP/1.1 messages and check every callback's
 * result -- a POST with headers and a Content-Length body fed one byte at a
 * time (fields and values split across calls must still join up), a
 * chunked response reassembled from its chunks, two pipelined requests on
 * one connection, keep-alive rules for 1.0 and 1.1, rejection of a
 * malformed message with the right errno, and the URL parser's fields. */
#include "common.h"
#include <stdlib.h>
#include <http_parser.h>

struct acc { char url[256], body[256], hdr[1024], status[64]; int msgs, hdrs_done, last; };
static void app(char *dst, size_t cap, const char *p, size_t n) {
  size_t l = strlen(dst); if (l + n >= cap) n = cap - 1 - l; memcpy(dst + l, p, n); dst[l + n] = 0; }
static int on_url(http_parser *p, const char *at, size_t n) { struct acc *a = p->data; app(a->url, sizeof a->url, at, n); return 0; }
static int on_status(http_parser *p, const char *at, size_t n) { struct acc *a = p->data; app(a->status, sizeof a->status, at, n); return 0; }
/* headers collected as "Field=value;" -- separators only when the callback kind changes */
static int on_hf(http_parser *p, const char *at, size_t n) { struct acc *a = p->data;
  if (a->last == 2) app(a->hdr, sizeof a->hdr, ";", 1);
  a->last = 1; app(a->hdr, sizeof a->hdr, at, n); return 0; }
static int on_hv(http_parser *p, const char *at, size_t n) { struct acc *a = p->data;
  if (a->last == 1) app(a->hdr, sizeof a->hdr, "=", 1);
  a->last = 2; app(a->hdr, sizeof a->hdr, at, n); return 0; }
static int on_hc(http_parser *p) { struct acc *a = p->data; a->hdrs_done++; if (a->last == 2) app(a->hdr, sizeof a->hdr, ";", 1); a->last = 0; return 0; }
static int on_body(http_parser *p, const char *at, size_t n) { struct acc *a = p->data; app(a->body, sizeof a->body, at, n); return 0; }
static int on_done(http_parser *p) { ((struct acc *)p->data)->msgs++; return 0; }

static http_parser_settings S;

int main(void) {
  QT_BEGIN("http-parser");
  printf("  info http_parser %lu.%lu.%lu\n", (http_parser_version() >> 16) & 255, (http_parser_version() >> 8) & 255,
         http_parser_version() & 255);
  http_parser_settings_init(&S);
  S.on_url = on_url; S.on_status = on_status; S.on_header_field = on_hf; S.on_header_value = on_hv;
  S.on_headers_complete = on_hc; S.on_body = on_body; S.on_message_complete = on_done;

  /* 1. POST, fed byte by byte */
  const char *req = "POST /api/v1/items?id=42 HTTP/1.1\r\nHost: example.org\r\n"
                    "Content-Type: application/json\r\nContent-Length: 14\r\n\r\n{\"name\":\"qnx\"}";
  http_parser p; struct acc a; memset(&a, 0, sizeof a);
  http_parser_init(&p, HTTP_REQUEST); p.data = &a;
  size_t used = 0;
  for (size_t i = 0; i < strlen(req); i++) used += http_parser_execute(&p, &S, req + i, 1);
  QT_CHECK(used == strlen(req) && HTTP_PARSER_ERRNO(&p) == HPE_OK, "byte-by-byte POST parsed without error");
  QT_CHECK(p.method == HTTP_POST && !strcmp(http_method_str(p.method), "POST"), "method POST");
  QT_CHECK(!strcmp(a.url, "/api/v1/items?id=42"), "URL /api/v1/items?id=42");
  QT_CHECK(p.http_major == 1 && p.http_minor == 1, "HTTP/1.1");
  QT_CHECK(!strcmp(a.hdr, "Host=example.org;Content-Type=application/json;Content-Length=14;"), "three headers, split pieces joined");
  QT_CHECK(!strcmp(a.body, "{\"name\":\"qnx\"}"), "14-byte body");
  QT_CHECK(a.msgs == 1 && a.hdrs_done == 1, "one message complete");
  QT_CHECK(http_should_keep_alive(&p), "HTTP/1.1 keeps the connection alive");

  /* 2. chunked response */
  const char *resp = "HTTP/1.1 404 Not Found\r\nTransfer-Encoding: chunked\r\n\r\n"
                     "5\r\nHello\r\n7\r\n, chunk\r\n3;ext=1\r\ned!\r\n0\r\nX-Trailer: t\r\n\r\n";
  memset(&a, 0, sizeof a); http_parser_init(&p, HTTP_RESPONSE); p.data = &a;
  used = http_parser_execute(&p, &S, resp, strlen(resp));
  QT_CHECK(used == strlen(resp) && HTTP_PARSER_ERRNO(&p) == HPE_OK, "chunked response parsed");
  QT_CHECK(p.status_code == 404 && !strcmp(a.status, "Not Found"), "status 404 Not Found");
  QT_CHECK(!strcmp(a.body, "Hello, chunked!"), "chunks reassembled: \"Hello, chunked!\"");
  QT_CHECK(strstr(a.hdr, "X-Trailer=t") != NULL, "trailer header seen");
  QT_CHECK(a.msgs == 1, "response complete");

  /* 3. two pipelined GETs, HTTP/1.0 without keep-alive */
  const char *two = "GET /a HTTP/1.1\r\nHost: h\r\n\r\nGET /b HTTP/1.0\r\n\r\n";
  memset(&a, 0, sizeof a); http_parser_init(&p, HTTP_REQUEST); p.data = &a;
  used = http_parser_execute(&p, &S, two, strlen(two));
  QT_CHECK(used == strlen(two) && a.msgs == 2 && !strcmp(a.url, "/a/b"), "two pipelined requests: /a then /b");
  QT_CHECK(p.http_minor == 0 && !http_should_keep_alive(&p), "HTTP/1.0 without keep-alive closes");

  /* 4. malformed */
  const char *bad = "GET / HTTP/1.1\r\nContent-Length: 1x\r\n\r\n";
  memset(&a, 0, sizeof a); http_parser_init(&p, HTTP_REQUEST); p.data = &a;
  used = http_parser_execute(&p, &S, bad, strlen(bad));
  QT_CHECK(HTTP_PARSER_ERRNO(&p) == HPE_INVALID_CONTENT_LENGTH && used < strlen(bad),
           "bad Content-Length rejected (HPE_INVALID_CONTENT_LENGTH)");
  printf("  info error: %s\n", http_errno_name(HTTP_PARSER_ERRNO(&p)));

  /* 5. URL parser */
  const char *url = "https://user@www.example.com:8443/path/to/x.html?q=1&r=2#frag";
  struct http_parser_url u; http_parser_url_init(&u);
  QT_CHECK(http_parser_parse_url(url, strlen(url), 0, &u) == 0, "URL parsed");
#define F(f) (int)u.field_data[f].len, url + u.field_data[f].off
  char buf[256];
  snprintf(buf, sizeof buf, "%.*s|%.*s|%.*s|%.*s|%.*s|%.*s", F(UF_SCHEMA), F(UF_USERINFO), F(UF_HOST), F(UF_PATH), F(UF_QUERY), F(UF_FRAGMENT));
  QT_CHECK(!strcmp(buf, "https|user|www.example.com|/path/to/x.html|q=1&r=2|frag"), "URL fields schema|user|host|path|query|fragment");
  QT_CHECK(u.port == 8443, "URL port 8443");
  QT_CHECK(http_parser_parse_url("http://[::1]:80x/", 17, 0, &u) != 0, "bad port rejected");
  QT_END();
}
