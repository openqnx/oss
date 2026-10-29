/* utf8proc: decoding, normalisation and character property lookup. */
#include "common.h"
#include <stdlib.h>
#include <utf8proc.h>
int main(void) {
  QT_BEGIN("utf8proc");
  QT_CHECK(utf8proc_version() != NULL, "version string");
  /* "gru" + combining diaeresis + sharp s + "e".  The escapes are split
   across string literals because \x is greedy: "\x9fe" would be parsed
   as one out-of-range escape rather than 0x9f followed by 'e'. */
  const utf8proc_uint8_t s[] = "gru\xcc\x88" "\xc3\x9f" "e";
  utf8proc_int32_t cp = 0;
  utf8proc_ssize_t used = utf8proc_iterate(s, -1, &cp);
  QT_CHECK(used == 1 && cp == 'g', "decode first code point");
  utf8proc_uint8_t *nfc = utf8proc_NFC(s);
  QT_CHECK(nfc != NULL, "NFC normalisation");
  /* u + combining diaeresis composes to U+00FC, so NFC is two bytes shorter. */
  QT_CHECK(nfc && strlen((char *)nfc) == strlen((char *)s) - 1, "NFC composed the diaeresis");
  free(nfc);
  QT_CHECK(utf8proc_category(0x00DF) == UTF8PROC_CATEGORY_LL, "sharp s is a lowercase letter");
  QT_CHECK(utf8proc_charwidth(0x4E00) == 2, "CJK ideograph is double width");
  QT_END();
}
