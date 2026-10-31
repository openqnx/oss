/* libpipeline: pipelines of in-process function commands (fork + pipes, no
 * external programs needed): a producer piped into an upper-casing filter,
 * output read line by line, exit statuses, pipeline_tostring, an input file
 * as stdin, peek/read, and one real program (sh -c) with an environment
 * variable set on the command. */
#include "common.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <pipeline.h>

static void producer(void *ud) {
  int n = *(int *)ud;
  for (int i = 1; i <= n; ++i) printf("line %d of %d\n", i, n);
  fflush(stdout);
}
static void upper(void *ud) {
  (void)ud;
  int c;
  while ((c = getchar()) != EOF) putchar(toupper(c));
  fflush(stdout);
}
static void fail3(void *ud) { (void)ud; fflush(stdout); _exit(3); }

int main(void) {
  QT_BEGIN("libpipeline");
  alarm(60);
  int five = 5;
  pipeline *p = pipeline_new();
  pipeline_command(p, pipecmd_new_function("producer", producer, NULL, &five));
  pipeline_command(p, pipecmd_new_function("upper", upper, NULL, NULL));
  pipeline_want_out(p, -1);
  char *desc = pipeline_tostring(p);
  printf("  info pipeline: %s\n", desc);
  QT_CHECK(desc && strcmp(desc, "producer | upper") == 0, "pipeline_tostring");
  free(desc);
  QT_CHECK(pipeline_get_ncommands(p) == 2, "two commands");
  pipeline_start(p);
  const char *peek = pipeline_peekline(p);
  QT_CHECK(peek && strcmp(peek, "LINE 1 OF 5\n") == 0, "peekline sees the first line");
  int lines = 0, ok = 1;
  const char *l;
  while ((l = pipeline_readline(p)) != NULL) {
    char want[32]; snprintf(want, sizeof want, "LINE %d OF 5\n", ++lines);
    if (strcmp(l, want) != 0) ok = 0;
  }
  QT_CHECK(lines == 5 && ok, "five upper-cased lines read through the pipe");
  QT_CHECK(pipeline_wait(p) == 0, "pipeline exit status 0");
  pipeline_free(p);

  p = pipeline_new();
  pipeline_command(p, pipecmd_new_function("producer", producer, NULL, &five));
  pipeline_command(p, pipecmd_new_function("fail3", fail3, NULL, NULL));
  pipeline_want_out(p, -1);
  pipeline_start(p);
  while (pipeline_readline(p)) {}
  int *st = NULL, nst = 0;
  int rc = pipeline_wait_all(p, &st, &nst);
  printf("  info wait_all rc %d, %d statuses\n", rc, nst);
  QT_CHECK(rc == 3 && nst == 2 && st && WIFEXITED(st[1]) && WEXITSTATUS(st[1]) == 3, "exit status 3 of the last command reported");
  free(st);
  pipeline_free(p);

  const char *inf = "/tmp/qnxport-libpipeline.txt";
  FILE *f = fopen(inf, "w");
  if (f) { fputs("from a file\nsecond\n", f); fclose(f); }
  p = pipeline_new_commands(pipecmd_new_function("upper", upper, NULL, NULL), (void *)0);
  pipeline_want_infile(p, inf);
  pipeline_want_out(p, -1);
  pipeline_start(p);
  const char *a = pipeline_readline(p);
  char first[32]; snprintf(first, sizeof first, "%s", a ? a : "");
  const char *b = pipeline_readline(p);
  QT_CHECK(strcmp(first, "FROM A FILE\n") == 0 && b && strcmp(b, "SECOND\n") == 0, "input file fed to the command");
  QT_CHECK(pipeline_wait(p) == 0, "file pipeline exits 0");
  pipeline_free(p);
  unlink(inf);

  pipecmd *sh = pipecmd_new_args("sh", "-c", "echo \"value=$QNX_PIPE_VAR\"", (void *)0);
  pipecmd_setenv(sh, "QNX_PIPE_VAR", "neutrino");
  p = pipeline_new_commands(sh, pipecmd_new_function("upper", upper, NULL, NULL), (void *)0);
  pipeline_want_out(p, -1);
  pipeline_start(p);
  l = pipeline_readline(p);
  printf("  info sh said: %s", l ? l : "(nothing)\n");
  QT_CHECK(l && strcmp(l, "VALUE=NEUTRINO\n") == 0, "external sh with a per-command environment variable");
  QT_CHECK(pipeline_wait(p) == 0, "sh pipeline exits 0");
  pipeline_free(p);
  QT_END();
}
