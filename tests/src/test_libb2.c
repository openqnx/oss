/* libb2: BLAKE2 known answers -- RFC 7693 appendix A/B ("abc" with
 * BLAKE2b-512 and BLAKE2s-256), the BLAKE2 authors' KAT vectors (input
 * 00 01 02 ... of length 0, 3 and 255; keyed with key 00 01 ... 3f/1f) for
 * BLAKE2b, BLAKE2s and the parallel BLAKE2bp and BLAKE2sp, incremental
 * hashing in odd-sized pieces equal to the one-shot hash, a shorter output
 * length, and rejection of an oversized key. On x86_64 libb2 is a "fat"
 * build: these run through whichever SIMD variant the CPU selected. */
#include "common.h"
#include <stdint.h>
#include <blake2.h>

static void hex(const uint8_t *p, size_t n, char *out) {
  for (size_t i = 0; i < n; i++) sprintf(out + 2 * i, "%02x", p[i]);
  out[2 * n] = 0;
}

int main(void) {
  QT_BEGIN("libb2");
  uint8_t in[256], key[64], out[64];
  char h[129];
  for (int i = 0; i < 256; i++) in[i] = (uint8_t)i;
  for (int i = 0; i < 64; i++) key[i] = (uint8_t)i;

  /* RFC 7693 */
  blake2b(out, "abc", NULL, 64, 3, 0); hex(out, 64, h);
  QT_CHECK(!strcmp(h, "ba80a53f981c4d0d6a2797b69f12f6e94c212f14685ac4b74b12bb6fdbffa2d1"
                      "7d87c5392aab792dc252d5de4533cc9518d38aa8dbf1925ab92386edd4009923"),
           "BLAKE2b-512(\"abc\") = RFC 7693 appendix A");
  blake2s(out, "abc", NULL, 32, 3, 0); hex(out, 32, h);
  QT_CHECK(!strcmp(h, "508c5e8c327c14e2e1a72ba34eeb452f37458b209ed63a294d999b4c86675982"),
           "BLAKE2s-256(\"abc\") = RFC 7693 appendix B");

  /* KAT (blake2-kat.h) */
  blake2b(out, in, NULL, 64, 0, 0); hex(out, 64, h);
  QT_CHECK(!strcmp(h, "786a02f742015903c6c6fd852552d272912f4740e15847618a86e217f71f5419"
                      "d25e1031afee585313896444934eb04b903a685b1448b755d56f701afe9be2ce"), "BLAKE2b(\"\")");
  blake2s(out, in, NULL, 32, 255, 0); hex(out, 32, h);
  QT_CHECK(!strcmp(h, "f03f5789d3336b80d002d59fdf918bdb775b00956ed5528e86aa994acb38fe2d"), "BLAKE2s(00..fe)");
  blake2b(out, in, key, 64, 3, 64); hex(out, 64, h);
  QT_CHECK(!strcmp(h, "33d0825dddf7ada99b0e7e307104ad07ca9cfd9692214f1561356315e784f3e5"
                      "a17e364ae9dbb14cb2036df932b77f4b292761365fb328de7afdc6d8998f5fc1"), "keyed BLAKE2b(00 01 02)");
  blake2b(out, in, key, 64, 255, 64); hex(out, 64, h);
  QT_CHECK(!strcmp(h, "142709d62e28fcccd0af97fad0f8465b971e82201dc51070faa0372aa43e9248"
                      "4be1c1e73ba10906d5d1853db6a4106e0a7bf9800d373d6dee2d46d62ef2a461"), "keyed BLAKE2b(00..fe)");
  blake2s(out, in, key, 32, 0, 32); hex(out, 32, h);
  QT_CHECK(!strcmp(h, "48a8997da407876b3d79c0d92325ad3b89cbb754d86ab71aee047ad345fd2c49"), "keyed BLAKE2s(\"\")");
  blake2s(out, in, key, 32, 255, 32); hex(out, 32, h);
  QT_CHECK(!strcmp(h, "3fb735061abc519dfe979e54c1ee5bfad0a9d858b3315bad34bde999efd724dd"), "keyed BLAKE2s(00..fe)");
  blake2bp(out, in, key, 64, 255, 64); hex(out, 64, h);
  QT_CHECK(!strcmp(h, "96fbcbb60bd313b8845033e5bc058a38027438572d7e7957f3684f6268aadd3a"
                      "d08d21767ed6878685331ba98571487e12470aad669326716e46667f69f8d7e8"), "keyed BLAKE2bp(00..fe)");
  blake2bp(out, in, key, 64, 3, 64); hex(out, 64, h);
  QT_CHECK(!strcmp(h, "30302c3fc999065d10dc982c8feef41bbb6642718f624af6e3eabea083e7fe78"
                      "5340db4b0897efff39cee1dc1eb737cd1eea0fe75384984e7d8f446faa683b80"), "keyed BLAKE2bp(00 01 02)");
  blake2sp(out, in, key, 32, 255, 32); hex(out, 32, h);
  QT_CHECK(!strcmp(h, "0c8a36597d7461c63a94732821c941856c668376606c86a52de0ee4104c615db"), "keyed BLAKE2sp(00..fe)");
  blake2sp(out, in, key, 32, 0, 32); hex(out, 32, h);
  QT_CHECK(!strcmp(h, "715cb13895aeb678f6124160bff21465b30f4f6874193fc851b4621043f09cc6"), "keyed BLAKE2sp(\"\")");

  /* incremental == one-shot */
  uint8_t one[64], inc[64];
  blake2b(one, in, key, 64, 255, 64);
  blake2b_state S; blake2b_init_key(&S, 64, key, 64);
  blake2b_update(&S, in, 1); blake2b_update(&S, in + 1, 127); blake2b_update(&S, in + 128, 127);
  blake2b_final(&S, inc, 64);
  QT_CHECK(!memcmp(one, inc, 64), "BLAKE2b in 1+127+127 byte pieces = one-shot");
  blake2sp_state SP; uint8_t ones[32], incs[32];
  blake2sp(ones, in, NULL, 32, 200, 0);
  blake2sp_init(&SP, 32); for (int i = 0; i < 200; i += 7) blake2sp_update(&SP, in + i, i + 7 <= 200 ? 7 : 200 - i);
  blake2sp_final(&SP, incs, 32);
  QT_CHECK(!memcmp(ones, incs, 32), "BLAKE2sp in 7-byte pieces = one-shot");

  /* a 32-byte BLAKE2b is a different hash (outlen is in the parameter block) */
  uint8_t b32[32]; blake2b(b32, "abc", NULL, 32, 3, 0); blake2b(out, "abc", NULL, 64, 3, 0);
  hex(b32, 32, h);
  QT_CHECK(memcmp(b32, out, 32) != 0, "BLAKE2b-256 is not a prefix of BLAKE2b-512");
  QT_CHECK(!strcmp(h, "bddd813c634239723171ef3fee98579b94964e3bb1cb3e427262c8c068d52319"), "BLAKE2b-256(\"abc\")");
  QT_CHECK(blake2s(out, "x", key, 32, 1, 33) < 0, "BLAKE2s rejects a 33-byte key");
  QT_END();
}
