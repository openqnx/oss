/* stringencoders (libmodpbase64): known answers for base64 (RFC 4648 test
 * vectors), base16, URL and JavaScript escaping, the number formatters and
 * ASCII case mapping; base85 and base64 round trips over all 256 byte
 * values, and the decoders' error return for invalid input (with the
 * lookup tables the build generated on the build machine). */
#include "common.h"
#include <modp_b64.h>
#include <modp_b16.h>
#include <modp_b85.h>
#include <modp_burl.h>
#include <modp_bjavascript.h>
#include <modp_numtoa.h>
#include <modp_ascii.h>

int main(void) {
  char out[1024], back[1024], all[256];
  size_t n; int i, ok;
  static const char *rfc[][2] = { {"", ""}, {"f", "Zg=="}, {"fo", "Zm8="}, {"foo", "Zm9v"},
                                   {"foob", "Zm9vYg=="}, {"fooba", "Zm9vYmE="}, {"foobar", "Zm9vYmFy"} };
  QT_BEGIN("stringencoders");
  ok = 1;
  for (i = 0; i < 7; i++) {
    n = modp_b64_encode(out, rfc[i][0], strlen(rfc[i][0]));
    ok &= n == strlen(rfc[i][1]) && strcmp(out, rfc[i][1]) == 0;
    n = modp_b64_decode(back, rfc[i][1], strlen(rfc[i][1]));
    ok &= n == strlen(rfc[i][0]) && memcmp(back, rfc[i][0], n) == 0;
  }
  QT_CHECK(ok, "base64 encode/decode: the 7 RFC 4648 test vectors");
  QT_CHECK(modp_b64_decode(back, "Zm9v!mFy", 8) == (size_t)-1, "base64 decode of an invalid character returns -1");
  for (i = 0; i < 256; i++) all[i] = (char)i;
  n = modp_b64_encode(out, all, 256);
  QT_CHECK(n == 344 && modp_b64_decode(back, out, n) == 256 && memcmp(back, all, 256) == 0, "base64 round trip of bytes 0..255");
  n = modp_b85_encode(out, all, 256);
  QT_CHECK(n == 320 && modp_b85_decode(back, out, n) == 256 && memcmp(back, all, 256) == 0, "base85 round trip of bytes 0..255");
  n = modp_b16_encode(out, "QNX\x01\xff", 5);
  QT_CHECK(n == 10 && strcmp(out, "514E5801FF") == 0, "base16 encode: 514E5801FF (upper case, as upstream documents)");
  QT_CHECK(modp_b16_decode(back, "514E58", 6) == 3 && memcmp(back, "QNX", 3) == 0, "base16 decode (upper case)");
  n = modp_burl_encode(out, "a b&c=d/\xe9", 9);
  QT_CHECK(n == 17 && strcmp(out, "a+b%26c%3Dd%2F%E9") == 0, "URL encode: a+b%26c%3Dd%2F%E9");
  n = modp_burl_decode(back, "a+b%26c%3d", 10);
  QT_CHECK(n == 6 && memcmp(back, "a b&c=", 6) == 0, "URL decode: + and %xx");
  modp_bjavascript_encode(out, "it's \"q\"\n", 9);
  QT_CHECK(strcmp(out, "it\\'s \\\"q\\\"\\n") == 0, "JavaScript string escaping");
  modp_itoa10(-2147483647 - 1, out);
  QT_CHECK(strcmp(out, "-2147483648") == 0, "modp_itoa10(INT32_MIN)");
  modp_ulitoa10(18446744073709551615ULL, out);
  QT_CHECK(strcmp(out, "18446744073709551615") == 0, "modp_ulitoa10(UINT64_MAX)");
  modp_dtoa(3.14159, out, 3);
  QT_CHECK(strcmp(out, "3.142") == 0, "modp_dtoa(3.14159, 3) = 3.142");
  modp_dtoa2(-0.5, out, 6);
  QT_CHECK(strcmp(out, "-0.5") == 0, "modp_dtoa2(-0.5, 6) = -0.5");
  strcpy(out, "Hello, QNX 8!"); modp_toupper(out, strlen(out));
  QT_CHECK(strcmp(out, "HELLO, QNX 8!") == 0, "modp_toupper");
  QT_END();
}
