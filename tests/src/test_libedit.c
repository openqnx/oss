/* libedit: the editline library behind many shells. Exercised through its
 * history and tokenizer APIs, which need no terminal. */
#include "common.h"
#include <stdlib.h>
#include <stdio.h>
#include <histedit.h>
int main(void) {
  QT_BEGIN("libedit");
  History *h = history_init();
  QT_CHECK(h != NULL, "history_init");
  HistEvent ev;
  history(h, &ev, H_SETSIZE, 100);
  history(h, &ev, H_ENTER, "build zlib");
  history(h, &ev, H_ENTER, "build zstd");
  history(h, &ev, H_ENTER, "run tests");
  QT_CHECK(history(h, &ev, H_GETSIZE) == 0 && ev.num == 3, "three entries stored");
  QT_CHECK(history(h, &ev, H_FIRST) == 0 && strcmp(ev.str, "run tests") == 0,
           "H_FIRST returns the newest entry");
  QT_CHECK(history(h, &ev, H_LAST) == 0 && strcmp(ev.str, "build zlib") == 0,
           "H_LAST returns the oldest entry");
  /* History search uses POSIX regex, which lives in QNX's own libregex. */
  QT_CHECK(history(h, &ev, H_PREV_STR, "build") == 0, "H_PREV_STR (regex search)");
  history_end(h);

  Tokenizer *t = tok_init(NULL);
  QT_CHECK(t != NULL, "tok_init");
  int argc = 0; const char **argv = NULL;
  QT_CHECK(tok_str(t, "qnxbuild -a aarch64le 'my pkg'", &argc, &argv) == 0, "tok_str");
  QT_CHECK(argc == 4, "four tokens");
  QT_CHECK(argv && strcmp(argv[3], "my pkg") == 0, "quoting is honoured");
  tok_end(t);
  QT_END();
}
