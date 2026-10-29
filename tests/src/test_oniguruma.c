/* oniguruma: compile a pattern, match it, read a capture group. */
#include "common.h"
#include <oniguruma.h>
int main(void) {
  QT_BEGIN("oniguruma");
  OnigEncoding encs[] = { ONIG_ENCODING_UTF8 };
  QT_CHECK(onig_initialize(encs, 1) == ONIG_NORMAL, "onig_initialize");
  static const UChar pat[] = "qnx-([0-9]+)\\.([0-9]+)";
  static const UChar subj[] = "building qnx-8.0 ports";
  regex_t *re = NULL;
  OnigErrorInfo einfo;
  int rc = onig_new(&re, pat, pat + strlen((const char *)pat),
                    ONIG_OPTION_DEFAULT, ONIG_ENCODING_UTF8,
                    ONIG_SYNTAX_PERL, &einfo);
  QT_CHECK(rc == ONIG_NORMAL, "onig_new compiles the pattern");
  OnigRegion *region = onig_region_new();
  const UChar *end = subj + strlen((const char *)subj);
  rc = onig_search(re, subj, end, subj, end, region, ONIG_OPTION_NONE);
  QT_CHECK(rc >= 0, "onig_search finds a match");
  QT_CHECK(region->num_regs == 3, "two capture groups");
  QT_CHECK(region->end[1] - region->beg[1] == 1 && subj[region->beg[1]] == '8',
           "first group captured \"8\"");
  onig_region_free(region, 1);
  onig_free(re);
  onig_end();
  QT_END();
}
