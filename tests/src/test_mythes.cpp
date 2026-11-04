/* mythes: a thesaurus written by the test (UTF-8 .dat, four entries, one
 * of them non-ASCII) and its index (.idx: encoding, entry count, sorted
 * "entry|offset" lines, the offsets computed here). Known answers: MyThes
 * reports the encoding, finds each entry with its meanings (definition: the
 * part of speech and the first synonym) and synonyms in order, finds nothing for unknown
 * words; th_gen_idx.pl (when perl is installed) writes the same index. */
#include "common.h"
#include <mythes.hxx>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <vector>
#include <algorithm>
#include <unistd.h>
#include <sys/wait.h>

static const char *DAT_LINES[] = {
  "UTF-8",
  "happy|2", "(adj)|glad|joyful|content", "(adj)|fortunate|lucky",
  "car|1", "(noun)|automobile|auto|motorcar",
  "fast|2", "(adj)|quick|rapid|speedy", "(verb)|abstain|diet",
  "caf\xc3\xa9|1", "(noun)|coffeehouse|coffee shop|bistro",
};

static std::string slurp(const std::string &p) {
  std::string s; FILE *f = fopen(p.c_str(), "rb");
  if (!f) return s;
  char b[4096]; size_t n;
  while ((n = fread(b, 1, sizeof b, f)) > 0) s.append(b, n);
  fclose(f);
  return s;
}

int main() {
  QT_BEGIN("mythes");
  char dir[] = "/tmp/qnxport-mythes.XXXXXX";
  QT_CHECK(mkdtemp(dir) != nullptr, "scratch directory");
  std::string dat = std::string(dir) + "/th_qx.dat", idx = std::string(dir) + "/th_qx.idx";
  /* the .dat, and the index of each entry's byte offset */
  std::string text; std::vector<std::pair<std::string, size_t>> ents;
  const size_t n = sizeof DAT_LINES / sizeof *DAT_LINES;
  for (size_t i = 0; i < n; i++) {
    const char *l = DAT_LINES[i];
    if (i > 0 && strchr(l, '|') && l[0] != '(') ents.push_back({std::string(l, strchr(l, '|') - l), text.size()});
    text += l; text += '\n';
  }
  std::sort(ents.begin(), ents.end());
  std::string itext = "UTF-8\n" + std::to_string(ents.size()) + "\n";
  for (auto &e : ents) itext += e.first + "|" + std::to_string(e.second) + "\n";
  FILE *f = fopen(dat.c_str(), "w"); if (f) { fputs(text.c_str(), f); fclose(f); }
  f = fopen(idx.c_str(), "w"); if (f) { fputs(itext.c_str(), f); fclose(f); }

  MyThes th(idx.c_str(), dat.c_str());
  const char *enc = th.get_th_encoding();
  QT_CHECK(enc && !strcmp(enc, "UTF-8"), "encoding UTF-8");

  struct want { const char *word; const char *meanings; };
  const want W[] = {
    /* the definition is "<pos> <first synonym>" (data_layout.txt: the
     * first synonym also describes the meaning) */
    { "happy", "(adj) glad:glad,joyful,content;(adj) fortunate:fortunate,lucky" },
    { "car", "(noun) automobile:automobile,auto,motorcar" },
    { "fast", "(adj) quick:quick,rapid,speedy;(verb) abstain:abstain,diet" },
    { "caf\xc3\xa9", "(noun) coffeehouse:coffeehouse,coffee shop,bistro" },
  };
  char what[512];
  for (auto &w : W) {
    mentry *pm = nullptr;
    int nm = th.Lookup(w.word, (int)strlen(w.word), &pm);
    std::string got;
    for (int i = 0; i < nm; i++) {
      if (i) got += ";";
      got += std::string(pm[i].defn ? pm[i].defn : "?") + ":";
      for (int j = 0; j < pm[i].count; j++) { if (j) got += ","; got += pm[i].psyns[j]; }
    }
    snprintf(what, sizeof what, "Lookup(%s) -> %s", w.word, w.meanings);
    QT_CHECK(got == w.meanings, what);
    if (got != w.meanings) printf("  info got %s\n", got.c_str());
    th.CleanUpAfterLookup(&pm, nm);
  }
  mentry *pm = nullptr;
  int nm = th.Lookup("unknownword", 11, &pm);
  QT_CHECK(nm == 0, "Lookup(unknownword) finds nothing");
  if (nm > 0) th.CleanUpAfterLookup(&pm, nm);
  nm = th.Lookup("ha", 2, &pm);
  QT_CHECK(nm == 0, "Lookup(ha), a prefix of an entry, finds nothing");
  if (nm > 0) th.CleanUpAfterLookup(&pm, nm);

  /* the index generator, if perl is there to run it */
  const char *pfx = getenv("QNX_PORT_PREFIX");
  std::string P = pfx ? pfx : "/usr/local";
  if (access((P + "/bin/perl").c_str(), X_OK) == 0) {
    std::string cmd = P + "/bin/perl " + P + "/bin/th_gen_idx.pl < " + dat + " > " + std::string(dir) + "/gen.idx";
    int st = system(cmd.c_str());
    std::string gen = slurp(std::string(dir) + "/gen.idx");
    QT_CHECK(st == 0 && gen == itext, "th_gen_idx.pl writes the same index");
    if (gen != itext) printf("  info th_gen_idx.pl wrote:\n%s", gen.c_str());
  } else {
    printf("  info perl not installed: th_gen_idx.pl not run\n");
  }
  std::string rm = std::string("rm -rf ") + dir;
  if (system(rm.c_str()) != 0) printf("  info could not remove %s\n", dir);
  QT_END();
}
