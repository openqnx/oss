/* aspell: no dictionary is packaged, so the test makes one. A tiny language
 * ("xx", ISO-8859-1) and a word list are compiled into a master dictionary
 * with `aspell create master`; then the libaspell API checks words, gets
 * suggestions for a misspelling, adds a word to the session, and the aspell
 * program's list/-a/dump modes run against the same dictionary. */
#include "common.h"
#include <aspell.h>
#include <stdlib.h>
#include <unistd.h>

static char dir[] = "/tmp/qnxport-aspell.XXXXXX";
static const char *ASPELL = "aspell";

static int run(const char *cmd, char *out, size_t n) {
  FILE *p = popen(cmd, "r");
  size_t used = 0;
  if (out && n) out[0] = 0;
  if (!p) return -1;
  char b[256];
  while (fgets(b, sizeof b, p)) {
    size_t l = strlen(b);
    if (out && used + l < n) { memcpy(out + used, b, l + 1); used += l; }
  }
  return pclose(p);
}

int main(void) {
  QT_BEGIN("aspell");
  if (access("/usr/local/bin/aspell", X_OK) == 0) ASPELL = "/usr/local/bin/aspell";
  if (!mkdtemp(dir)) { QT_CHECK(0, "mkdtemp"); QT_END(); }
  char path[512], cmd[2048], out[4096];
  snprintf(path, sizeof path, "%s/xx.dat", dir);
  FILE *f = fopen(path, "w");
  if (f) { fputs("name xx\ncharset iso8859-1\n", f); fclose(f); }
  snprintf(path, sizeof path, "%s/words.txt", dir);
  f = fopen(path, "w");
  if (f) { fputs("apple\nbanana\ncherry\nkernel\nmicrokernel\nqnx\nneutrino\n", f); fclose(f); }

  snprintf(cmd, sizeof cmd, "%s --version 2>&1", ASPELL);
  int rc = run(cmd, out, sizeof out);
  printf("  info %s", out);
  QT_CHECK(rc == 0 && strstr(out, "Aspell 0.60.8.1"), "aspell --version: 0.60.8.1");

  snprintf(cmd, sizeof cmd, "%s --lang=xx --local-data-dir=%s create master %s/xx.rws < %s/words.txt 2>&1",
           ASPELL, dir, dir, dir);
  rc = run(cmd, out, sizeof out);
  if (rc != 0) printf("  info create: %s\n", out);
  snprintf(path, sizeof path, "%s/xx.rws", dir);
  QT_CHECK(rc == 0 && access(path, R_OK) == 0, "aspell create master: xx.rws compiled");

  /* The C API */
  AspellConfig *cfg = new_aspell_config();
  aspell_config_replace(cfg, "master", path);
  aspell_config_replace(cfg, "local-data-dir", dir);
  aspell_config_replace(cfg, "encoding", "utf-8");
  AspellCanHaveError *ret = new_aspell_speller(cfg);
  delete_aspell_config(cfg);
  if (aspell_error_number(ret) != 0) {
    printf("  info new_aspell_speller: %s\n", aspell_error_message(ret));
    QT_CHECK(0, "new_aspell_speller");
    delete_aspell_can_have_error(ret);
  } else {
    AspellSpeller *sp = to_aspell_speller(ret);
    QT_CHECK(1, "new_aspell_speller on the compiled dictionary");
    AspellConfig *sc = aspell_config_clone(aspell_speller_config(sp));
    const char *lang = aspell_config_retrieve(sc, "lang");
    printf("  info speller lang: %s\n", lang ? lang : "(null)");
    QT_CHECK(lang && strcmp(lang, "xx") == 0, "speller language is xx");
    delete_aspell_config(sc);
    QT_CHECK(aspell_speller_check(sp, "banana", -1) == 1, "check(\"banana\") == correct");
    QT_CHECK(aspell_speller_check(sp, "microkernel", -1) == 1, "check(\"microkernel\") == correct");
    QT_CHECK(aspell_speller_check(sp, "bananna", -1) == 0, "check(\"bananna\") == misspelled");
    QT_CHECK(aspell_speller_check(sp, "linux", -1) == 0, "check(\"linux\") == unknown");
    const AspellWordList *sug = aspell_speller_suggest(sp, "bananna", -1);
    AspellStringEnumeration *e = aspell_word_list_elements(sug);
    const char *w, *first = NULL; int found = 0, cnt = 0;
    char firstbuf[64] = "";
    while ((w = aspell_string_enumeration_next(e)) != NULL) {
      if (!first) { snprintf(firstbuf, sizeof firstbuf, "%s", w); first = firstbuf; }
      if (strcmp(w, "banana") == 0) found = 1;
      cnt++;
    }
    delete_aspell_string_enumeration(e);
    printf("  info %d suggestion(s) for bananna, first: %s\n", cnt, first ? first : "(none)");
    QT_CHECK(found && first && strcmp(first, "banana") == 0, "suggest(\"bananna\") -> \"banana\" first");
    QT_CHECK(aspell_speller_add_to_session(sp, "photon", -1) == 1, "add_to_session(\"photon\")");
    QT_CHECK(aspell_speller_check(sp, "photon", -1) == 1, "session word now correct");
    delete_aspell_speller(sp);
  }

  /* The aspell program */
  snprintf(cmd, sizeof cmd, "echo 'apple bananna cherry qnz kernel' | %s --master=%s --local-data-dir=%s list 2>&1",
           ASPELL, path, dir);
  rc = run(cmd, out, sizeof out);
  printf("  info aspell list: %s", out);
  QT_CHECK(rc == 0 && strcmp(out, "bananna\nqnz\n") == 0, "aspell list: exactly bananna, qnz");
  snprintf(cmd, sizeof cmd, "echo 'cherri' | %s --master=%s --local-data-dir=%s -a 2>&1", ASPELL, path, dir);
  rc = run(cmd, out, sizeof out);
  QT_CHECK(rc == 0 && strstr(out, "& cherri ") && strstr(out, ": cherry"), "aspell -a: '& cherri ... : cherry'");
  snprintf(cmd, sizeof cmd, "%s --master=%s --local-data-dir=%s dump master 2>&1 | grep -c .", ASPELL, path, dir);
  rc = run(cmd, out, sizeof out);
  QT_CHECK(rc == 0 && atoi(out) == 7, "aspell dump master: 7 words");

  snprintf(cmd, sizeof cmd, "rm -rf '%s'", dir);
  system(cmd);
  QT_END();
}
