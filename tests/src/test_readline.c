/* readline: the history and line-buffer APIs, which are usable without a tty. */
#include "common.h"
#include <stdlib.h>
#include <stdio.h>
#include <readline/readline.h>
#include <readline/history.h>
int main(void) {
  QT_BEGIN("readline");
  printf("  info readline %s\n", rl_library_version);
  QT_CHECK(rl_readline_version > 0, "rl_readline_version");
  using_history();
  add_history("build zlib");
  add_history("build zstd");
  add_history("run tests");
  HIST_ENTRY **h = history_list();
  QT_CHECK(h != NULL, "history_list");
  QT_CHECK(history_length == 3, "three entries in the history");
  QT_CHECK(h && strcmp(h[0]->line, "build zlib") == 0, "first entry");
  int pos = history_search_prefix("build", -1);
  QT_CHECK(pos >= 0, "history_search_prefix found a match");
  HIST_ENTRY *removed = remove_history(0);
  QT_CHECK(removed != NULL && history_length == 2, "remove_history");
  if (removed) free_history_entry(removed);

  /* Word expansion is the piece shells rely on. */
  char *expansion = NULL;
  int rc = history_expand((char *)"!!", &expansion);
  QT_CHECK(rc >= 0 && expansion != NULL, "history_expand(\"!!\")");
  if (expansion) { printf("  info \"!!\" -> %s\n", expansion); free(expansion); }

  /* The line buffer API used by completion callbacks. */
  rl_initialize();
  QT_CHECK(rl_line_buffer != NULL, "rl_line_buffer is allocated");
  rl_insert_text("build ");
  QT_CHECK(strcmp(rl_line_buffer, "build ") == 0, "rl_insert_text");
  QT_CHECK(rl_point == 6, "rl_point tracks the insertion");
  rl_delete_text(0, rl_end);
  rl_point = 0;
  QT_CHECK(rl_end == 0, "rl_delete_text cleared the buffer");
  clear_history();
  QT_CHECK(history_length == 0, "clear_history");
  QT_END();
}
