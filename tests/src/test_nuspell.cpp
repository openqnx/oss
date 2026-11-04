/* nuspell: the Hunspell-format test dictionary of test_hunspell (suffixes,
 * a prefix, REP, a non-ASCII stem) loaded by libnuspell, from a path and
 * from streams. Known answers: correct and affixed forms accepted, case
 * variants as Hunspell defines them (Title and UPPER case of a lower-case
 * word are correct, mixed case is not), misspellings rejected, suggestions
 * (REP f->ph, a transposition, a wrong affix); the nuspell program checks
 * a file and prints "* OK" / "& Wrong: <w>. How about: <suggestions>".
 * The files are written without a byte order mark, and once with one:
 * nuspell strips a BOM (QNX's libc++ once made every BOM-less file fail). */
#include "common.h"
#include <nuspell/dictionary.hxx>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <sstream>
#include <string>
#include <vector>
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

static std::string join(const std::vector<std::string> &v) {
  std::string s;
  for (auto &x : v) { if (!s.empty()) s += ", "; s += x; }
  return s;
}
static int run(const std::string &cmd, std::string &out) {
  out.clear();
  FILE *p = popen(cmd.c_str(), "r");
  if (!p) return -1;
  char b[4096]; size_t n;
  while ((n = fread(b, 1, sizeof b, p)) > 0) out.append(b, n);
  int st = pclose(p);
  return WIFEXITED(st) ? WEXITSTATUS(st) : -1;
}

int main() {
  QT_BEGIN("nuspell");
  char dir[] = "/tmp/qnxport-nuspell.XXXXXX";
  QT_CHECK(mkdtemp(dir) != nullptr, "scratch directory");
  std::string aff = std::string(dir) + "/qx.aff", dic = std::string(dir) + "/qx.dic", in = std::string(dir) + "/in.txt";
  FILE *f = fopen(aff.c_str(), "w"); if (f) { fputs(AFF, f); fclose(f); }
  f = fopen(dic.c_str(), "w"); if (f) { fputs(DIC, f); fclose(f); }
  f = fopen(in.c_str(), "w");
  if (f) { fputs("cats dogs tried unhappy waited caf\xc3\xa9s\ncatz tryed grafs dgo unwait trys\n", f); fclose(f); }

  nuspell::Dictionary d;
  bool loaded = false;
  try { d.load_aff_dic(aff); loaded = true; }
  catch (const std::exception &e) { printf("  info %s\n", e.what()); }
  QT_CHECK(loaded, "load_aff_dic(path)");
  if (!loaded) QT_END();
  char what[512];
  for (const char *w : { "cat", "cats", "dogs", "tried", "unhappy", "waited", "waits", "graphs", "caf\xc3\xa9s",
                         "Cats", "CATS", "CAF\xc3\x89S" }) {
    snprintf(what, sizeof what, "\"%s\" is correct", w); QT_CHECK(d.spell(w), what);
  }
  for (const char *w : { "catz", "tryed", "trys", "unwait", "grafs", "dgo", "happys", "cAts" }) {
    snprintf(what, sizeof what, "\"%s\" is misspelled", w); QT_CHECK(!d.spell(w), what);
  }
  struct { const char *w, *s; } S[] = { { "grafs", "graphs" }, { "dgo", "dog" }, { "tryed", "tried, try" } };
  for (auto &s : S) {
    std::vector<std::string> v; d.suggest(s.w, v);
    snprintf(what, sizeof what, "suggest(%s) = [%s] (got [%s])", s.w, s.s, join(v).c_str());
    QT_CHECK(join(v) == s.s, what);
  }
  /* the same dictionary from streams */
  std::istringstream ia(AFF), id(DIC);
  nuspell::Dictionary d2;
  bool ok2 = false;
  try { d2.load_aff_dic(ia, id); ok2 = d2.spell("waited") && !d2.spell("waitted"); } catch (...) {}
  QT_CHECK(ok2, "load_aff_dic(istream, istream): waited / not waitted");
  /* files that start with a UTF-8 byte order mark (stripped by nuspell) */
  std::string baff = std::string(dir) + "/bom.aff", bdic = std::string(dir) + "/bom.dic";
  f = fopen(baff.c_str(), "w"); if (f) { fputs("\xEF\xBB\xBF", f); fputs(AFF, f); fclose(f); }
  f = fopen(bdic.c_str(), "w"); if (f) { fputs("\xEF\xBB\xBF", f); fputs(DIC, f); fclose(f); }
  nuspell::Dictionary d3;
  bool ok3 = false;
  try { d3.load_aff_dic(baff); ok3 = d3.spell("cats") && d3.spell("caf\xc3\xa9") && !d3.spell("catz"); }
  catch (const std::exception &e) { printf("  info %s\n", e.what()); }
  QT_CHECK(ok3, "a dictionary with a UTF-8 BOM loads: cats, caf\xc3\xa9 / not catz");

  /* the program */
  const char *pfx = getenv("QNX_PORT_PREFIX");
  std::string P = pfx ? pfx : "/usr/local", out;
  int rc = run(P + "/bin/nuspell -d " + aff + " " + in + " 2>/dev/null", out);
  const std::string want =
    "* OK\n* OK\n* OK\n* OK\n* OK\n* OK\n\n"
    "& Wrong: catz. How about: cat, cats\n& Wrong: tryed. How about: tried, try\n"
    "& Wrong: grafs. How about: graphs\n& Wrong: dgo. How about: dog\n"
    "& Wrong: unwait. How about: wait\n& Wrong: trys. How about: try\n\n";
  snprintf(what, sizeof what, "nuspell -d qx.aff in.txt: six OK, six wrong with suggestions (rc %d)", rc);
  QT_CHECK(rc == 0 && out == want, what);
  if (out != want) printf("  info got:\n%s", out.c_str());
  std::string rm = std::string("rm -rf ") + dir;
  if (system(rm.c_str()) != 0) printf("  info could not remove %s\n", dir);
  QT_END();
}
