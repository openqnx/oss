/* pcre2: compile, match and substitute with the 8-bit API. */
#include "common.h"
#define PCRE2_CODE_UNIT_WIDTH 8
#include <pcre2.h>
int main(void) {
  QT_BEGIN("pcre2");
  int errnum; PCRE2_SIZE erroff;
  pcre2_code *re = pcre2_compile((PCRE2_SPTR) "(?<maj>\\d+)\\.(?<min>\\d+)",
                                 PCRE2_ZERO_TERMINATED, 0, &errnum, &erroff, NULL);
  QT_CHECK(re != NULL, "pcre2_compile");
  pcre2_match_data *md = pcre2_match_data_create_from_pattern(re, NULL);
  PCRE2_SPTR subj = (PCRE2_SPTR) "QNX SDP 8.0 aarch64le";
  int rc = pcre2_match(re, subj, PCRE2_ZERO_TERMINATED, 0, 0, md, NULL);
  QT_CHECK(rc == 3, "match with two capture groups");
  PCRE2_UCHAR buf[16]; PCRE2_SIZE blen = sizeof buf;
  QT_CHECK(pcre2_substring_copy_byname(md, (PCRE2_SPTR) "maj", buf, &blen) == 0 &&
           strcmp((char *)buf, "8") == 0, "named group \"maj\" == 8");
  PCRE2_UCHAR out[64]; PCRE2_SIZE olen = sizeof out;
  rc = pcre2_substitute(re, subj, PCRE2_ZERO_TERMINATED, 0,
                        PCRE2_SUBSTITUTE_GLOBAL, NULL, NULL,
                        (PCRE2_SPTR) "$1_$2", PCRE2_ZERO_TERMINATED, out, &olen);
  QT_CHECK(rc > 0 && strcmp((char *)out, "QNX SDP 8_0 aarch64le") == 0, "pcre2_substitute");
  pcre2_match_data_free(md);
  pcre2_code_free(re);
  QT_END();
}
