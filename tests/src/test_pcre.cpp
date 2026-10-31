// PCRE 8.45: known matches through the 8-bit API (captures, named groups,
// UTF-8 + Unicode properties, JIT study), the 16-bit library, the POSIX
// wrapper and the C++ wrapper; then the pcregrep and pcretest programs.
#include "common.h"
#include <pcre.h>
#include <pcreposix.h>
#include <pcrecpp.h>
#include <stdlib.h>
#include <unistd.h>
#include <string>

static std::string run(const std::string &cmd, int *rc) {
  std::string out;
  FILE *p = popen(cmd.c_str(), "r");
  if (!p) { *rc = -1; return out; }
  char b[512];
  while (fgets(b, sizeof b, p)) out += b;
  *rc = pclose(p);
  return out;
}

int main() {
  QT_BEGIN("pcre");
  printf("  info pcre_version: %s\n", pcre_version());
  QT_CHECK(strncmp(pcre_version(), "8.45", 4) == 0, "pcre_version() 8.45");
  int jit = 0, utf8 = 0, ucp = 0;
  pcre_config(PCRE_CONFIG_JIT, &jit);
  pcre_config(PCRE_CONFIG_UTF8, &utf8);
  pcre_config(PCRE_CONFIG_UNICODE_PROPERTIES, &ucp);
  printf("  info config: jit=%d utf8=%d unicode-properties=%d\n", jit, utf8, ucp);
  QT_CHECK(utf8 && ucp, "built with UTF-8 and Unicode properties");

  const char *err = NULL; int eo = 0, ov[30];
  pcre *re = pcre_compile("(\\d{4})-(\\d{2})-(\\d{2})", 0, &err, &eo, NULL);
  QT_CHECK(re != NULL, "pcre_compile date pattern");
  const char *subj = "released on 2021-06-15, patched later";
  int rc = re ? pcre_exec(re, NULL, subj, (int)strlen(subj), 0, 0, ov, 30) : -99;
  QT_CHECK(rc == 4 && ov[0] == 12 && ov[1] == 22, "pcre_exec: match at offset 12..22, 3 groups");
  const char *g = NULL;
  if (rc == 4 && pcre_get_substring(subj, ov, rc, 2, &g) >= 0) {
    QT_CHECK(strcmp(g, "06") == 0, "group 2 == \"06\"");
    pcre_free_substring(g);
  } else QT_CHECK(0, "pcre_get_substring");
  if (re) pcre_free(re);

  re = pcre_compile("(?<user>\\w+)@(?<host>[\\w.]+)", 0, &err, &eo, NULL);
  const char *mail = "contact: dev@qnx.example.org";
  rc = re ? pcre_exec(re, NULL, mail, (int)strlen(mail), 0, 0, ov, 30) : -99;
  const char *h = NULL;
  if (rc > 0 && pcre_get_named_substring(re, mail, ov, rc, "host", &h) >= 0) {
    QT_CHECK(strcmp(h, "qnx.example.org") == 0, "named group host == qnx.example.org");
    pcre_free_substring(h);
  } else QT_CHECK(0, "named group match");
  if (re) pcre_free(re);

  re = pcre_compile("^\\p{Lu}\\p{Ll}+\\s\\p{Greek}+$", PCRE_UTF8, &err, &eo, NULL);
  const char *u = "\xc3\x9c" "ber \xce\xb1\xce\xb2\xce\xb3";  // "Über αβγ"
  rc = re ? pcre_exec(re, NULL, u, (int)strlen(u), 0, 0, ov, 30) : -99;
  QT_CHECK(rc == 1, "UTF-8 + \\p{Lu}\\p{Ll}+ \\p{Greek}+ matches");
  if (re) pcre_free(re);

  re = pcre_compile("caf\\x{e9}", PCRE_UTF8 | PCRE_CASELESS, &err, &eo, NULL);
  const char *c = "CAF\xc3\x89";  // "CAFÉ"
  rc = re ? pcre_exec(re, NULL, c, (int)strlen(c), 0, 0, ov, 30) : -99;
  QT_CHECK(rc == 1, "caseless UTF-8 match of e-acute vs E-acute");
  if (re) pcre_free(re);

  re = pcre_compile("a(?=b)|x+?y", 0, &err, &eo, NULL);
  rc = re ? pcre_exec(re, NULL, "zzac ab", 7, 0, 0, ov, 30) : -99;
  QT_CHECK(rc == 1 && ov[0] == 5, "lookahead: 'a' before 'b' found at 5");
  if (re) pcre_free(re);

  re = pcre_compile("(?:[a-z]+\\d)+$", 0, &err, &eo, NULL);
  pcre_extra *ex = re ? pcre_study(re, PCRE_STUDY_JIT_COMPILE, &err) : NULL;
  int isjit = 0;
  if (ex) pcre_fullinfo(re, ex, PCRE_INFO_JIT, &isjit);
  printf("  info JIT-compiled: %d\n", isjit);
  if (jit) QT_CHECK(isjit == 1, "pcre_study(JIT) compiled the pattern");
  int hits = 0;
  const char *js[] = {"abc1def2", "abc1def", "x9", "9x"};
  for (int i = 0; i < 4; i++)
    if (re && pcre_exec(re, ex, js[i], (int)strlen(js[i]), 0, 0, ov, 30) >= 0) hits |= 1 << i;
  QT_CHECK(hits == 0x5, "studied/JIT pattern matches exactly abc1def2 and x9");
  if (ex) pcre_free_study(ex);
  if (re) pcre_free(re);

  // 16-bit library
  PCRE_UCHAR16 pat16[] = {'b', '+', 0};
  PCRE_UCHAR16 s16[] = {'a', 'b', 'b', 'b', 'c', 0};
  pcre16 *r16 = pcre16_compile(pat16, 0, &err, &eo, NULL);
  rc = r16 ? pcre16_exec(r16, NULL, s16, 5, 0, 0, ov, 30) : -99;
  QT_CHECK(rc == 1 && ov[0] == 1 && ov[1] == 4, "pcre16: b+ in \"abbbc\" at 1..4");
  if (r16) pcre16_free(r16);

  // POSIX wrapper
  regex_t px; regmatch_t pm[2];
  int prc = regcomp(&px, "(\\d+)(?= apples)", 0);
  QT_CHECK(prc == 0, "pcreposix regcomp with \\d and lookahead");
  if (prc == 0) {
    int x = regexec(&px, "I have 12 pears and 34 apples", 2, pm, 0);
    QT_CHECK(x == 0 && pm[1].rm_so == 20 && pm[1].rm_eo == 22, "pcreposix regexec: \"34\" at 20..22");
    regfree(&px);
  }

  // C++ wrapper
  int num = 0; std::string word;
  QT_CHECK(pcrecpp::RE("(\\w+):(\\d+)").FullMatch("ruby:1234", &word, &num) && word == "ruby" && num == 1234,
           "pcrecpp FullMatch extracts word and int");
  std::string s = "yabba dabba doo";
  pcrecpp::RE("b+").GlobalReplace("d", &s);
  QT_CHECK(s == "yada dada doo", "pcrecpp GlobalReplace");

  // Programs
  char dir[] = "/tmp/qnxport-pcre.XXXXXX";
  if (!mkdtemp(dir)) { QT_CHECK(0, "mkdtemp"); QT_END(); }
  std::string d = dir, in = d + "/in.txt", tin = d + "/t.txt";
  FILE *f = fopen(in.c_str(), "w");
  if (f) { fputs("alpha 12\nbeta\ngamma 345 6\n", f); fclose(f); }
  std::string bin = access("/usr/local/bin/pcregrep", X_OK) == 0 ? "/usr/local/bin/" : "";
  std::string out = run(bin + "pcregrep -o '\\d+' " + in, &rc);
  printf("  info pcregrep -o: %s", out.c_str());
  QT_CHECK(rc == 0 && out == "12\n345\n6\n", "pcregrep -o '\\d+' prints 12, 345, 6");
  out = run(bin + "pcregrep -c -v '\\d' " + in, &rc);
  QT_CHECK(rc == 0 && out == "1\n", "pcregrep -c -v counts 1 line");
  f = fopen(tin.c_str(), "w");
  if (f) { fputs("/(a+)(b)?c/\n    xaac\n    nope\n", f); fclose(f); }
  out = run(bin + "pcretest -q " + tin, &rc);
  printf("  info pcretest: %s\n", out.substr(0, out.find('\n')).c_str());
  QT_CHECK(out.find(" 0: aac") != std::string::npos && out.find(" 1: aa") != std::string::npos &&
           out.find("No match") != std::string::npos, "pcretest: 0: aac / 1: aa / No match");
  unlink(in.c_str()); unlink(tin.c_str()); rmdir(dir);
  QT_END();
}
