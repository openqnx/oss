/* hunspell: spell checking with a small dictionary written by the test (an
 * affix file with suffix, prefix, REP and morphological fields, and seven
 * stems, one of them non-ASCII). Library (libhunspell-1.7, C API): known
 * words and their affixed forms are accepted, misspellings rejected,
 * suggestions come from REP and from the keyboard/edit distance, stems and
 * morphological analyses are the dictionary's, a word added at run time is
 * accepted. Program (hunspell): -l lists exactly the misspelled words of a
 * file, -a (ispell pipe mode) answers '*' and '&' lines, -s stems. */
#include "common.h"
#include <hunspell/hunspell.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

static const char *AFF =
  "SET UTF-8\n"
  "TRY esianrtolcdugmphbyfvkwzESIANRTOLCDUGMPHBYFVKWZ\n"
  "KEY qwertyuiop|asdfghjkl|zxcvbnm\n"
  "REP 1\nREP f ph\n"
  "SFX S Y 1\nSFX S 0 s . is:plural\n"
  "SFX D Y 2\nSFX D 0 ed [^y] is:past\nSFX D y ied y is:past\n"
  "PFX U Y 1\nPFX U 0 un . ip:un\n";
static const char *DIC =
  "7\ncat/S po:noun\ndog/S po:noun\ntry/D po:verb\nhappy/U po:adj\ngraph/S po:noun\n"
  "wait/DS po:verb\ncaf\xc3\xa9/S po:noun\n";

static char dir[64];
static int write_file(const char *name, const char *text) {
  char p[128]; snprintf(p, sizeof p, "%s/%s", dir, name);
  FILE *f = fopen(p, "w");
  if (!f) return 0;
  fputs(text, f);
  return fclose(f) == 0;
}
static int run(const char *cmd, char *out, size_t n) {
  out[0] = 0;
  FILE *p = popen(cmd, "r");
  if (!p) return -1;
  size_t len = fread(out, 1, n - 1, p);
  out[len] = 0;
  int st = pclose(p);
  return WIFEXITED(st) ? WEXITSTATUS(st) : -1;
}
static int list_has(char **l, int n, const char *w) {
  for (int i = 0; i < n; i++) if (!strcmp(l[i], w)) return 1;
  return 0;
}
static void join(char **l, int n, char *buf, size_t sz) {
  buf[0] = 0;
  for (int i = 0; i < n; i++) { if (i) strncat(buf, "|", sz - strlen(buf) - 1); strncat(buf, l[i], sz - strlen(buf) - 1); }
}

int main(void) {
  QT_BEGIN("hunspell");
  snprintf(dir, sizeof dir, "/tmp/qnxport-hunspell.XXXXXX");
  QT_CHECK(mkdtemp(dir) != NULL, "scratch directory");
  QT_CHECK(write_file("qx.aff", AFF) && write_file("qx.dic", DIC), "dictionary qx.aff / qx.dic written");
  char aff[128], dic[128], what[1200], buf[1024];
  snprintf(aff, sizeof aff, "%s/qx.aff", dir); snprintf(dic, sizeof dic, "%s/qx.dic", dir);

  Hunhandle *h = Hunspell_create(aff, dic);
  QT_CHECK(h != NULL, "Hunspell_create");
  if (!h) QT_END();
  QT_CHECK(!strcmp(Hunspell_get_dic_encoding(h), "UTF-8"), "dictionary encoding UTF-8");
  const char *good[] = { "cat", "cats", "dogs", "tried", "unhappy", "waited", "waits", "graphs", "caf\xc3\xa9s" };
  const char *bad[]  = { "catz", "tryed", "trys", "unwait", "grafs", "dgo", "happys" };
  for (size_t i = 0; i < sizeof good / sizeof *good; i++) {
    snprintf(what, sizeof what, "\"%s\" is correct", good[i]); QT_CHECK(Hunspell_spell(h, good[i]) != 0, what);
  }
  for (size_t i = 0; i < sizeof bad / sizeof *bad; i++) {
    snprintf(what, sizeof what, "\"%s\" is misspelled", bad[i]); QT_CHECK(Hunspell_spell(h, bad[i]) == 0, what);
  }
  char **l; int n;
  n = Hunspell_suggest(h, &l, "grafs"); join(l, n, buf, sizeof buf);
  snprintf(what, sizeof what, "suggest(grafs) has graphs (REP f->ph): [%s]", buf); QT_CHECK(list_has(l, n, "graphs"), what);
  Hunspell_free_list(h, &l, n);
  n = Hunspell_suggest(h, &l, "dgo"); join(l, n, buf, sizeof buf);
  snprintf(what, sizeof what, "suggest(dgo) has dog: [%s]", buf); QT_CHECK(list_has(l, n, "dog"), what);
  Hunspell_free_list(h, &l, n);
  n = Hunspell_stem(h, &l, "tried"); join(l, n, buf, sizeof buf);
  snprintf(what, sizeof what, "stem(tried) = try: [%s]", buf); QT_CHECK(n == 1 && !strcmp(l[0], "try"), what);
  Hunspell_free_list(h, &l, n);
  n = Hunspell_stem(h, &l, "unhappy"); join(l, n, buf, sizeof buf);
  snprintf(what, sizeof what, "stem(unhappy) = happy: [%s]", buf); QT_CHECK(n == 1 && !strcmp(l[0], "happy"), what);
  Hunspell_free_list(h, &l, n);
  n = Hunspell_analyze(h, &l, "cats"); join(l, n, buf, sizeof buf);
  snprintf(what, sizeof what, "analyze(cats): st:cat po:noun is:plural [%s]", buf);
  QT_CHECK(n == 1 && strstr(l[0], "st:cat") && strstr(l[0], "po:noun") && strstr(l[0], "is:plural"), what);
  Hunspell_free_list(h, &l, n);
  n = Hunspell_generate(h, &l, "dog", "cats"); join(l, n, buf, sizeof buf);
  snprintf(what, sizeof what, "generate(dog like cats) = dogs: [%s]", buf); QT_CHECK(n == 1 && !strcmp(l[0], "dogs"), what);
  Hunspell_free_list(h, &l, n);
  QT_CHECK(Hunspell_spell(h, "qnx") == 0, "\"qnx\" unknown before Hunspell_add");
  Hunspell_add_with_affix(h, "qnx", "cat");
  QT_CHECK(Hunspell_spell(h, "qnx") && Hunspell_spell(h, "qnxs"), "after Hunspell_add_with_affix(qnx like cat): qnx, qnxs");
  Hunspell_destroy(h);

  /* the program */
  write_file("in.txt", "cats dogs tried unhappy waited caf\xc3\xa9s\ncatz tryed grafs dgo unwait trys\n");
  char cmd[512];
  snprintf(cmd, sizeof cmd, "hunspell -d %s/qx -l %s/in.txt 2>&1", dir, dir);
  int rc = run(cmd, buf, sizeof buf);
  snprintf(what, sizeof what, "hunspell -l lists the six misspelled words (rc %d)", rc);
  QT_CHECK(rc == 0 && !strcmp(buf, "catz\ntryed\ngrafs\ndgo\nunwait\ntrys\n"), what);
  if (strcmp(buf, "catz\ntryed\ngrafs\ndgo\nunwait\ntrys\n")) printf("  info got: %s\n", buf);
  snprintf(cmd, sizeof cmd, "echo 'cats tried grafs dgo caf\xc3\xa9' | hunspell -d %s/qx -a 2>&1", dir);
  rc = run(cmd, buf, sizeof buf);
  const char *want_a = "*\n*\n& grafs 1 11: graphs\n& dgo 1 17: dog\n*\n\n";
  char *body = strchr(buf, '\n');           /* after the version banner */
  snprintf(what, sizeof what, "hunspell -a (ispell pipe): banner, then * * & & * (rc %d)", rc);
  QT_CHECK(rc == 0 && !strncmp(buf, "@(#) International Ispell", 25) && body && !strcmp(body + 1, want_a), what);
  if (!body || strcmp(body + 1, want_a)) printf("  info got: %s\n", buf);
  snprintf(cmd, sizeof cmd, "echo 'cats waited unhappy' | hunspell -d %s/qx -s 2>&1", dir);
  rc = run(cmd, buf, sizeof buf);
  QT_CHECK(rc == 0 && !strcmp(buf, "cats cat\n\nwaited wait\n\nunhappy happy\n\n"), "hunspell -s stems");
  snprintf(cmd, sizeof cmd, "rm -rf %s", dir);
  if (system(cmd) != 0) printf("  info could not remove %s\n", dir);
  QT_END();
}
