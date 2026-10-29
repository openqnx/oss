/* libunistring: UTF conversion, case mapping and normalisation. */
#include "common.h"
#include <stdlib.h>
#include <unistr.h>
#include <uniconv.h>
#include <unicase.h>
#include <uninorm.h>
#include <unistdio.h>
int main(void) {
  QT_BEGIN("libunistring");
  const uint8_t *s = (const uint8_t *) "Stra" "\xc3\x9f" "e";   /* "Straße" */
  QT_CHECK(u8_strlen(s) == 7, "u8_strlen counts bytes");
  QT_CHECK(u8_mbsnlen(s, u8_strlen(s)) == 6, "u8_mbsnlen counts characters");
  size_t len = 0;
  uint8_t *up = u8_toupper(s, u8_strlen(s), "de", NULL, NULL, &len);
  QT_CHECK(up != NULL, "u8_toupper");
  /* German uppercasing expands ß to SS, so the result is one character longer. */
  QT_CHECK(up && len == 7 && memcmp(up, "STRASSE", 7) == 0, "ß uppercases to SS");
  free(up);
  size_t nlen = 0;
  const uint8_t *decomposed = (const uint8_t *) "u\xcc\x88";  /* u + combining diaeresis */
  uint8_t *nfc = u8_normalize(UNINORM_NFC, decomposed, 3, NULL, &nlen);
  QT_CHECK(nfc && nlen == 2 && nfc[0] == 0xC3 && nfc[1] == 0xBC, "NFC composes to U+00FC");
  free(nfc);
  size_t ulen = 0;
  uint32_t *u32 = u8_to_u32(s, u8_strlen(s) + 1, NULL, &ulen);
  QT_CHECK(u32 && u32[4] == 0x00DF, "UTF-8 to UTF-32 keeps ß at U+00DF");
  free(u32);
  QT_END();
}
