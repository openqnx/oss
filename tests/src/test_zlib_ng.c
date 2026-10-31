/* zlib-ng (native zng_ API): known CRC-32 / Adler-32 vectors, a one-shot
 * compress/uncompress round trip, a streaming gzip-wrapped deflate/inflate
 * of a larger buffer, and inflating a fixed raw-deflate stream produced by
 * reference zlib (so the decoder is checked against foreign output). */
#include "common.h"
#include <stdlib.h>
#include <stdint.h>
#include <zlib-ng.h>
int main(void) {
  QT_BEGIN("zlib-ng");
  const char *v = zlibng_version();
  printf("  info zlib-ng %s (header %s)\n", v ? v : "(null)", ZLIBNG_VERSION);
  QT_CHECK(v && strcmp(v, ZLIBNG_VERSION) == 0, "library version matches header");

  const uint8_t *abc = (const uint8_t *)"123456789";
  uint32_t crc = zng_crc32(0, abc, 9);
  printf("  info crc32(\"123456789\") = %08x\n", (unsigned)crc);
  QT_CHECK(crc == 0xCBF43926u, "CRC-32 check value 0xCBF43926");
  uint32_t ad = zng_adler32(1, (const uint8_t *)"Wikipedia", 9);
  QT_CHECK(ad == 0x11E60398u, "Adler-32(\"Wikipedia\") = 0x11E60398");

  /* one-shot */
  static uint8_t src[64 * 1024], cmp[80 * 1024], out[64 * 1024];
  for (size_t i = 0; i < sizeof src; i++) src[i] = (uint8_t)("QNX zlib-ng round trip "[i % 23] ^ (i / 4096));
  size_t cl = sizeof cmp;
  QT_CHECK(zng_compress2(cmp, &cl, src, sizeof src, 9) == Z_OK, "zng_compress2 level 9");
  printf("  info %zu -> %zu bytes\n", sizeof src, cl);
  QT_CHECK(cl < sizeof src / 4, "repetitive data compresses well");
  size_t ol = sizeof out;
  QT_CHECK(zng_uncompress(out, &ol, cmp, cl) == Z_OK && ol == sizeof src && memcmp(out, src, ol) == 0,
           "zng_uncompress restores the input exactly");

  /* streaming, gzip wrapper (windowBits 31) */
  zng_stream s; memset(&s, 0, sizeof s);
  QT_CHECK(zng_deflateInit2(&s, 6, Z_DEFLATED, 31, 8, Z_DEFAULT_STRATEGY) == Z_OK, "deflateInit2 (gzip)");
  s.next_in = src; s.avail_in = sizeof src; s.next_out = cmp; s.avail_out = sizeof cmp;
  int r = zng_deflate(&s, Z_FINISH);
  size_t gl = s.total_out;
  zng_deflateEnd(&s);
  QT_CHECK(r == Z_STREAM_END && cmp[0] == 0x1f && cmp[1] == 0x8b, "gzip stream with magic 1f 8b");
  uint32_t trailer_crc = cmp[gl - 8] | cmp[gl - 7] << 8 | cmp[gl - 6] << 16 | (uint32_t)cmp[gl - 5] << 24;
  QT_CHECK(trailer_crc == zng_crc32(0, src, sizeof src), "gzip trailer CRC matches zng_crc32 of input");
  memset(&s, 0, sizeof s);
  QT_CHECK(zng_inflateInit2(&s, 31) == Z_OK, "inflateInit2 (gzip)");
  s.next_in = cmp; s.avail_in = (uint32_t)gl; s.next_out = out; s.avail_out = sizeof out;
  r = zng_inflate(&s, Z_FINISH);
  QT_CHECK(r == Z_STREAM_END && s.total_out == sizeof src && memcmp(out, src, sizeof src) == 0,
           "streaming gzip inflate round trip");
  zng_inflateEnd(&s);

  /* raw deflate of "hello hello hello hello\n" from reference zlib */
  static const uint8_t ref[] = { 0xcb, 0x48, 0xcd, 0xc9, 0xc9, 0x57, 0xc8, 0x40, 0x27, 0xb9, 0x00 };
  memset(&s, 0, sizeof s);
  zng_inflateInit2(&s, -15);
  char txt[64] = { 0 };
  s.next_in = (uint8_t *)ref; s.avail_in = sizeof ref; s.next_out = (uint8_t *)txt; s.avail_out = sizeof txt - 1;
  r = zng_inflate(&s, Z_FINISH);
  zng_inflateEnd(&s);
  printf("  info inflated reference stream: %d bytes\n", (int)s.total_out);
  if (r != Z_STREAM_END) printf("  info inflate returned %d\n", r);
  QT_CHECK(r == Z_STREAM_END && strcmp(txt, "hello hello hello hello\n") == 0, "inflates a reference raw-deflate stream");

  /* corrupted input is rejected */
  memcpy(out, cmp, gl); out[gl / 2] ^= 0x55; out[gl - 8] ^= 0xff;
  memset(&s, 0, sizeof s); zng_inflateInit2(&s, 31);
  static uint8_t junk[64 * 1024];
  s.next_in = out; s.avail_in = (uint32_t)gl; s.next_out = junk; s.avail_out = sizeof junk;
  r = zng_inflate(&s, Z_FINISH);
  zng_inflateEnd(&s);
  QT_CHECK(r == Z_DATA_ERROR || r == Z_BUF_ERROR, "corrupted gzip stream is rejected");
  QT_END();
}
