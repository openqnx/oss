/* libjson (vincenthz): the streaming SAX-style parser and the printer -- an
 * event sequence with typed values, \u escapes decoded to UTF-8, a parse ->
 * print round trip that reproduces compact JSON, byte-at-a-time feeding,
 * C comments when allowed, and the error codes for bad input and nesting
 * limits. */
#include "common.h"
#include <stdio.h>
#include <json.h>

struct ev { int types[64]; char data[64][32]; int n; };
static int collect(void *ud, int type, const char *data, uint32_t len) {
  struct ev *e = ud;
  if (e->n < 64) {
    e->types[e->n] = type;
    snprintf(e->data[e->n], sizeof e->data[0], "%.*s", data ? (int)len : 0, data ? data : "");
    e->n++;
  }
  return 0;
}
struct sbuf { char s[512]; size_t n; };
static int sink(void *ud, const char *s, uint32_t len) {
  struct sbuf *b = ud;
  if (b->n + len < sizeof b->s) { memcpy(b->s + b->n, s, len); b->n += len; b->s[b->n] = 0; }
  return 0;
}
static int reprint(void *ud, int type, const char *data, uint32_t len) { return json_print_raw(ud, type, data, len); }
static int parse_all(json_config *cfg, const char *txt, json_parser_callback cb, void *ud) {
  json_parser p;
  if (json_parser_init(&p, cfg, cb, ud)) return -1;
  int rc = json_parser_string(&p, txt, (uint32_t)strlen(txt), NULL);
  if (rc == 0 && !json_parser_is_done(&p)) rc = -2;
  json_parser_free(&p);
  return rc;
}

int main(void) {
  QT_BEGIN("libjson");
  const char *doc = "{\"name\":\"qnx\",\"n\":[1,2.5,-3e2],\"ok\":true,\"nil\":null,\"esc\":\"a\\\"b\\u00e9\"}";
  static struct ev e;
  QT_CHECK(parse_all(NULL, doc, collect, &e) == 0, "document parses");
  static const int want[] = {JSON_OBJECT_BEGIN, JSON_KEY, JSON_STRING, JSON_KEY, JSON_ARRAY_BEGIN, JSON_INT, JSON_FLOAT,
                             JSON_FLOAT, JSON_ARRAY_END, JSON_KEY, JSON_TRUE, JSON_KEY, JSON_NULL, JSON_KEY, JSON_STRING,
                             JSON_OBJECT_END};
  int same = e.n == 16;
  for (int i = 0; same && i < 16; ++i) same = e.types[i] == want[i];
  printf("  info %d events\n", e.n);
  QT_CHECK(same, "event sequence: object, keys, string, int, floats, true, null");
  QT_CHECK(e.n == 16 && strcmp(e.data[1], "name") == 0 && strcmp(e.data[2], "qnx") == 0 && strcmp(e.data[5], "1") == 0 &&
           strcmp(e.data[7], "-3e2") == 0, "key and value text");
  QT_CHECK(e.n == 16 && strcmp(e.data[14], "a\"b\xc3\xa9") == 0, "escapes decoded (\\\" and \\u00e9 -> UTF-8)");

  static struct sbuf out;
  json_printer pr;
  json_print_init(&pr, sink, &out);
  const char *compact = "{\"k\":[1,2,{\"x\":false}],\"s\":\"v\"}";
  QT_CHECK(parse_all(NULL, compact, reprint, &pr) == 0, "parse into the raw printer");
  json_print_free(&pr);
  printf("  info reprinted %s\n", out.s);
  QT_CHECK(strcmp(out.s, compact) == 0, "parse -> print_raw reproduces the compact text");

  memset(&e, 0, sizeof e);
  json_parser p;
  json_parser_init(&p, NULL, collect, &e);
  int rc = 0;
  for (const char *c = "[true, false, \"x y\"]"; *c && !rc; ++c) rc = json_parser_char(&p, (unsigned char)*c);
  QT_CHECK(rc == 0 && json_parser_is_done(&p) && e.n == 5 && e.types[3] == JSON_STRING && strcmp(e.data[3], "x y") == 0,
           "byte-at-a-time feeding");
  json_parser_free(&p);

  json_config cfg; memset(&cfg, 0, sizeof cfg);
  cfg.allow_c_comments = 1;
  memset(&e, 0, sizeof e);
  QT_CHECK(parse_all(&cfg, "[1, /* two */ 2]", collect, &e) == 0 && e.n == 4, "C comments accepted when enabled");
  memset(&e, 0, sizeof e);
  QT_CHECK(parse_all(NULL, "[1, /* two */ 2]", collect, &e) == JSON_ERROR_COMMENT_NOT_ALLOWED, "comments rejected by default");
  memset(&e, 0, sizeof e);
  QT_CHECK(parse_all(NULL, "{\"a\" 1}", collect, &e) > 0, "missing colon is an error");
  memset(&cfg, 0, sizeof cfg);
  cfg.max_nesting = 2;
  memset(&e, 0, sizeof e);
  QT_CHECK(parse_all(&cfg, "[[[1]]]", collect, &e) == JSON_ERROR_NESTING_LIMIT, "nesting limit enforced");
  memset(&e, 0, sizeof e);
  QT_CHECK(parse_all(NULL, "[1,2", collect, &e) == -2, "truncated input is not done");
  QT_END();
}
