/* libfyaml: YAML known answers -- parse a document with a mapping,
 * sequences, anchors/aliases, a block literal and a tagged scalar; read
 * values by path and with fy_document_scanf; emit it back and re-parse it
 * (round trip: fy_node_compare says the trees are equal); emit as one-line
 * flow and as JSON with exact expected text; build a document from code;
 * and report a syntax error instead of a document. The tmpfile() memory
 * stream (ports/libfyaml/patches/0002) is what token/event text goes
 * through. */
#include "common.h"
#include <stdlib.h>
#include <libfyaml.h>

static const char *YAML =
  "name: qnx-oss\n"
  "version: 8.0\n"
  "arches: [x86_64, aarch64le]\n"
  "defaults: &d\n"
  "  timeout: 30\n"
  "  retries: 3\n"
  "service:\n"
  "  <<: *d\n"
  "  host: localhost\n"
  "motd: |\n"
  "  line one\n"
  "  line two\n"
  "count: !!int \"42\"\n";

static const char *scalar(struct fy_document *d, const char *path) {
  static char buf[256];
  struct fy_node *n = fy_node_by_path(fy_document_root(d), path, (size_t)-1, FYNWF_DONT_FOLLOW);
  size_t len; const char *s = n ? fy_node_get_scalar(n, &len) : NULL;
  if (!s) return "(none)";
  snprintf(buf, sizeof buf, "%.*s", (int)len, s);
  return buf;
}

int main(void) {
  QT_BEGIN("libfyaml");
  printf("  info libfyaml %s\n", fy_library_version());
  struct fy_parse_cfg cfg = { .flags = FYPCF_DEFAULT_DOC };
  struct fy_document *d = fy_document_build_from_string(&cfg, YAML, FY_NT);
  QT_CHECK(d != NULL, "document parsed");
  if (!d) QT_END();
  QT_CHECK(!strcmp(scalar(d, "/name"), "qnx-oss"), "/name = qnx-oss");
  QT_CHECK(!strcmp(scalar(d, "/arches/1"), "aarch64le"), "/arches/1 = aarch64le");
  QT_CHECK(fy_node_sequence_item_count(fy_node_by_path(fy_document_root(d), "/arches", FY_NT, FYNWF_DONT_FOLLOW)) == 2,
           "/arches has 2 items");
  QT_CHECK(!strcmp(scalar(d, "/motd"), "line one\nline two\n"), "block literal keeps its newlines");
  QT_CHECK(!strcmp(scalar(d, "/service/host"), "localhost"), "/service/host = localhost");
  QT_CHECK(!strcmp(scalar(d, "/defaults/timeout"), "30"), "anchored /defaults/timeout = 30");
  size_t tl; const char *tag = fy_node_get_tag(fy_node_by_path(fy_document_root(d), "/count", FY_NT, FYNWF_DONT_FOLLOW), &tl);
  QT_CHECK(tag && tl >= 3 && !strncmp(tag + tl - 3, "int", 3), "/count carries the !!int tag");
  int timeout = 0, count = 0; char name[32] = "";
  QT_CHECK(fy_document_scanf(d, "/defaults/timeout %d /name %31s /count %d", &timeout, name, &count) == 3
           && timeout == 30 && !strcmp(name, "qnx-oss") && count == 42, "fy_document_scanf: 30, qnx-oss, 42");

  /* round trip: emit (block), parse again, compare trees */
  char *text = fy_emit_document_to_string(d, FYECF_DEFAULT);
  QT_CHECK(text && strstr(text, "name: qnx-oss") && strstr(text, "*d"), "emitted text keeps keys and the alias");
  struct fy_document *d2 = text ? fy_document_build_from_string(&cfg, text, FY_NT) : NULL;
  QT_CHECK(d2 && fy_node_compare(fy_document_root(d), fy_document_root(d2)), "round trip: re-parsed tree equals the original");
  free(text);
  fy_document_destroy(d2);

  /* flow / JSON with exact text */
  struct fy_document *s = fy_document_build_from_string(&cfg, "a: 1\nb: [x, \"y z\"]\nc: {k: v}\n", FY_NT);
  char *flow = fy_emit_document_to_string(s, FYECF_MODE_FLOW_ONELINE);
  char *json = fy_emit_document_to_string(s, FYECF_MODE_JSON_ONELINE);
  printf("  info flow: %s  info json: %s", flow ? flow : "-\n", json ? json : "-\n");
  QT_CHECK(flow && !strcmp(flow, "{a: 1, b: [x, \"y z\"], c: {k: v}}\n"), "one-line flow emission");
  QT_CHECK(json && !strcmp(json, "{\"a\": 1, \"b\": [\"x\", \"y z\"], \"c\": {\"k\": \"v\"}}\n"), "one-line JSON emission");
  free(flow); free(json); fy_document_destroy(s);

  /* build from code */
  struct fy_document *b = fy_document_create(&cfg);
  struct fy_node *m = fy_node_create_mapping(b), *seq = fy_node_create_sequence(b);
  fy_node_sequence_append(seq, fy_node_create_scalar_copy(b, "1", FY_NT));
  fy_node_sequence_append(seq, fy_node_create_scalar_copy(b, "2", FY_NT));
  fy_node_mapping_append(m, fy_node_create_scalar_copy(b, "list", FY_NT), seq);
  fy_node_mapping_append(m, fy_node_create_scalar_copy(b, "key", FY_NT), fy_node_create_scalar_copy(b, "value", FY_NT));
  fy_document_set_root(b, m);
  char *bt = fy_emit_document_to_string(b, FYECF_MODE_FLOW_ONELINE);
  QT_CHECK(bt && !strcmp(bt, "{list: [1, 2], key: value}\n"), "document built from code emits {list: [1, 2], key: value}");
  free(bt); fy_document_destroy(b);

  /* syntax error */
  struct fy_parse_cfg quiet = { .flags = FYPCF_QUIET };
  QT_CHECK(fy_document_build_from_string(&quiet, "a: [1, 2\nb: }\n", FY_NT) == NULL, "unterminated flow sequence is an error");

  fy_document_destroy(d);
  QT_END();
}
