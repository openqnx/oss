#include "common.h"
#include <dlfcn.h>
#include <string.h>
#include <tree_sitter/api.h>
/* The tree-sitter grammars as Neovim loads them: lib/tree-sitter/<lang>.so,
 * opened with dlopen, the tree_sitter_<lang>() entry point, a sample parsed.
 * Known answers: the root node's type, the first statement's type, no
 * syntax errors. */
struct g { const char *lang, *src, *root, *first; };
static const struct g gs[] = {
  { "bash",   "echo hello | tr a-z A-Z\n", "program",     "pipeline" },
  { "python", "x = [i * 2 for i in range(3)]\n", "module", "expression_statement" },
  { "cmake",  "project(demo C)\nadd_executable(demo main.c)\n", "source_file", "normal_command" },
  { "make",   "all: main.o\n\tcc -o demo main.o\n", "makefile", "rule" },
};
int main(void) {
  QT_BEGIN("tree-sitter-grammars");
  const char *dirs[] = { "/usr/local/lib/tree-sitter", "/data/oss/usr/local/lib/tree-sitter" };
  for (unsigned i = 0; i < sizeof gs / sizeof gs[0]; i++) {
    char path[256], sym[64], what[128]; void *h = NULL;
    for (int d = 0; d < 2 && !h; d++) { snprintf(path, sizeof path, "%s/%s.so", dirs[d], gs[i].lang); h = dlopen(path, RTLD_NOW); }
    snprintf(what, sizeof what, "%s.so loads", gs[i].lang);
    QT_CHECK(h != NULL, what);
    if (!h) { printf("  info %s\n", dlerror()); continue; }
    snprintf(sym, sizeof sym, "tree_sitter_%s", gs[i].lang);
    const TSLanguage *(*fn)(void) = (const TSLanguage *(*)(void))dlsym(h, sym);
    snprintf(what, sizeof what, "%s() found", sym); QT_CHECK(fn != NULL, what);
    if (!fn) continue;
    TSParser *p = ts_parser_new();
    snprintf(what, sizeof what, "%s: language accepted (ABI)", gs[i].lang);
    QT_CHECK(ts_parser_set_language(p, fn()), what);
    TSTree *t = ts_parser_parse_string(p, NULL, gs[i].src, (uint32_t)strlen(gs[i].src));
    TSNode root = ts_tree_root_node(t);
    TSNode first = ts_node_named_child(root, 0);
    printf("  info %s: %s > %s\n", gs[i].lang, ts_node_type(root), ts_node_type(first));
    snprintf(what, sizeof what, "%s: root is %s, first is %s", gs[i].lang, gs[i].root, gs[i].first);
    QT_CHECK(strcmp(ts_node_type(root), gs[i].root) == 0 && strcmp(ts_node_type(first), gs[i].first) == 0, what);
    snprintf(what, sizeof what, "%s: no syntax errors", gs[i].lang);
    QT_CHECK(!ts_node_has_error(root), what);
    ts_tree_delete(t); ts_parser_delete(p);
  }
  QT_END();
}
