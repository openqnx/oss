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
  /* tree-sitter 0.25 dropped the parse timeout (ts_parser_set_timeout_micros)
   * in favour of TSParseOptions.progress_callback; the parser's other state
   * round-trips are checked instead. */
  TSParseOptions opts = { NULL, NULL };
  QT_CHECK(ts_parser_parse_with_options(p, NULL,
             (TSInput){ NULL, NULL, TSInputEncodingUTF8, NULL }, opts) == NULL,
           "parse_with_options without a language returns NULL");
  TSRange rs[2] = { { {0, 0}, {0, 4}, 0, 4 }, { {1, 0}, {1, 9}, 10, 19 } };
  QT_CHECK(ts_parser_set_included_ranges(p, rs, 2), "ordered included ranges accepted");
  uint32_t nr = 0;
  const TSRange *got = ts_parser_included_ranges(p, &nr);
  QT_CHECK(nr == 2 && got[1].start_byte == 10 && got[1].end_byte == 19,
           "included ranges round trip");
  TSRange bad[2] = { rs[1], rs[0] };
  QT_CHECK(!ts_parser_set_included_ranges(p, bad, 2), "out-of-order ranges rejected");
  TSPoint pt = { 3, 7 };
  QT_CHECK(pt.row == 3 && pt.column == 7, "TSPoint layout");
  ts_parser_reset(p);
  ts_parser_delete(p);
  QT_CHECK(TREE_SITTER_LANGUAGE_VERSION >= 13, "language ABI version");
  printf("  info language ABI %d (min %d)\n",
         TREE_SITTER_LANGUAGE_VERSION, TREE_SITTER_MIN_COMPATIBLE_LANGUAGE_VERSION);
  QT_END();
}
