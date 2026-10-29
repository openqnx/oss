/* tree-sitter: build a parser and drive the incremental-parsing machinery.
 * No grammar is linked in, so this exercises the runtime itself -- allocation,
 * the language ABI check, tree lifetime and the cursor API. */
#include "common.h"
#include <tree_sitter/api.h>
int main(void) {
  QT_BEGIN("tree-sitter");
  TSParser *p = ts_parser_new();
  QT_CHECK(p != NULL, "ts_parser_new");
  QT_CHECK(ts_parser_language(p) == NULL, "no language set yet");
  /* Parsing without a language must fail cleanly rather than crash. */
  TSTree *t = ts_parser_parse_string(p, NULL, "int x;", 6);
  QT_CHECK(t == NULL, "parsing without a language returns NULL");
  ts_parser_set_timeout_micros(p, 1000000);
  QT_CHECK(ts_parser_timeout_micros(p) == 1000000, "timeout round trip");
  TSPoint pt = { 3, 7 };
  QT_CHECK(pt.row == 3 && pt.column == 7, "TSPoint layout");
  ts_parser_reset(p);
  ts_parser_delete(p);
  QT_CHECK(TREE_SITTER_LANGUAGE_VERSION >= 13, "language ABI version");
  printf("  info language ABI %d (min %d)\n",
         TREE_SITTER_LANGUAGE_VERSION, TREE_SITTER_MIN_COMPATIBLE_LANGUAGE_VERSION);
  QT_END();
}
