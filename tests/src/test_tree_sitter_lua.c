/* tree-sitter-lua: the grammar as Neovim loads it -- lib/tree-sitter/<lang>.so through
 * dlopen and its tree_sitter_<lang>() entry point -- driven by the tree-sitter
 * runtime. Lua.
 * Known answers: the full syntax tree of a sample, a query's captures, and
 * (where the grammar can reject anything) an error flagged in malformed input. */
#include "common.h"
#include <dlfcn.h>
#include <stdlib.h>
#include <string.h>
#include <tree_sitter/api.h>

struct spec { const char *lang, *src, *sexp, *query, *caps, *broken; };
static const char *grammar_dir(void) {
  const char *e = getenv("TS_GRAMMAR_DIR");   /* for running it elsewhere */
  return e ? e : "/usr/local/lib/tree-sitter";
}
/* Captured node texts of a query over the tree, joined with ','. */
static int run_query(const TSLanguage *l, TSNode root, const char *src, const char *q, char *buf, size_t n) {
  uint32_t eo = 0; TSQueryError et = TSQueryErrorNone;
  TSQuery *qq = ts_query_new(l, q, (uint32_t)strlen(q), &eo, &et);
  if (!qq) { printf("  info query error %d at offset %u\n", (int)et, eo); return 0; }
  TSQueryCursor *c = ts_query_cursor_new(); ts_query_cursor_exec(c, qq, root);
  TSQueryMatch m; uint32_t ci; buf[0] = 0; size_t len = 0;
  while (ts_query_cursor_next_capture(c, &m, &ci)) {
    TSNode nd = m.captures[ci].node;
    uint32_t s = ts_node_start_byte(nd), e = ts_node_end_byte(nd);
    int w = snprintf(buf + len, n - len, "%s%.*s", len ? "," : "", (int)(e - s), src + s);
    if (w < 0 || (size_t)w >= n - len) break;
    len += (size_t)w;
  }
  ts_query_cursor_delete(c); ts_query_delete(qq);
  return 1;
}
static void check_lang(const struct spec *g) {
  char path[512], sym[96], what[160];
  snprintf(path, sizeof path, "%s/%s.so", grammar_dir(), g->lang);
  void *h = dlopen(path, RTLD_NOW);
  snprintf(what, sizeof what, "%s.so loads (dlopen)", g->lang); QT_CHECK(h != NULL, what);
  if (!h) { const char *e = dlerror(); printf("  info %s: %s\n", path, e ? e : "?"); return; }
  snprintf(sym, sizeof sym, "tree_sitter_%s", g->lang);
  const TSLanguage *(*fn)(void) = (const TSLanguage *(*)(void))dlsym(h, sym);
  snprintf(what, sizeof what, "%s() entry point", sym); QT_CHECK(fn != NULL, what);
  if (!fn) return;
  const TSLanguage *l = fn();
  uint32_t abi = ts_language_abi_version(l);
  printf("  info %s: ABI %u, %u symbols, %u fields\n", g->lang, abi, ts_language_symbol_count(l), ts_language_field_count(l));
  TSParser *p = ts_parser_new();
  snprintf(what, sizeof what, "%s: ABI %u accepted by the runtime (%d..%d)", g->lang, abi,
           TREE_SITTER_MIN_COMPATIBLE_LANGUAGE_VERSION, TREE_SITTER_LANGUAGE_VERSION);
  QT_CHECK(abi >= TREE_SITTER_MIN_COMPATIBLE_LANGUAGE_VERSION && abi <= TREE_SITTER_LANGUAGE_VERSION &&
           ts_parser_set_language(p, l), what);
  TSTree *t = ts_parser_parse_string(p, NULL, g->src, (uint32_t)strlen(g->src));
  snprintf(what, sizeof what, "%s: sample parsed", g->lang); QT_CHECK(t != NULL, what);
  if (!t) { ts_parser_delete(p); return; }
  TSNode root = ts_tree_root_node(t);
  snprintf(what, sizeof what, "%s: no syntax errors in the sample", g->lang);
  QT_CHECK(!ts_node_has_error(root), what);
  char *sx = ts_node_string(root);
  printf("  info %s tree: %s\n", g->lang, sx ? sx : "(null)");
  snprintf(what, sizeof what, "%s: syntax tree is the expected one", g->lang);
  QT_CHECK(sx && strcmp(sx, g->sexp) == 0, what);
  if (sx && strcmp(sx, g->sexp) != 0) printf("  info want %s\n", g->sexp);
  free(sx);
  snprintf(what, sizeof what, "%s: whole sample covered (0..%u bytes)", g->lang, (unsigned)strlen(g->src));
  QT_CHECK(ts_node_start_byte(root) == 0 && ts_node_end_byte(root) == strlen(g->src), what);
  char caps[512];
  int qok = run_query(l, root, g->src, g->query, caps, sizeof caps);
  printf("  info query %s -> [%s]\n", g->query, qok ? caps : "");
  snprintf(what, sizeof what, "%s: query captures [%s]", g->lang, g->caps);
  QT_CHECK(qok && strcmp(caps, g->caps) == 0, what);
  ts_tree_delete(t);
  if (g->broken) {
    t = ts_parser_parse_string(p, NULL, g->broken, (uint32_t)strlen(g->broken));
    snprintf(what, sizeof what, "%s: malformed input is flagged (has_error)", g->lang);
    QT_CHECK(t && ts_node_has_error(ts_tree_root_node(t)), what);
    if (t) ts_tree_delete(t);
  }
  ts_parser_delete(p);
}
static const struct spec specs[] = {
  { "lua",
    "local function sq(x) return x * x end\nprint(sq(7))\n",
    "(chunk local_declaration: (function_declaration name: (identifier) parameters: (parameters name: (identifier)) body: (block (return_statement (expression_list (binary_expression left: (identifier) right: (identifier)))))) (function_call name: (identifier) arguments: (arguments (function_call name: (identifier) arguments: (arguments (number))))))",
    "(function_call name: (identifier) @call)", "print,sq",
    "local function (x return end end\n" },
};
int main(void) {
  QT_BEGIN("tree-sitter-lua");
  printf("  info tree-sitter runtime: language ABI %d..%d\n", TREE_SITTER_MIN_COMPATIBLE_LANGUAGE_VERSION, TREE_SITTER_LANGUAGE_VERSION);
  for (unsigned i = 0; i < sizeof specs / sizeof specs[0]; i++) check_lang(&specs[i]);
  QT_END();
}
