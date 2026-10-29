/* libdeflate: gzip compress and decompress a buffer. */
#include "common.h"
#include <libdeflate.h>
int main(void) {
  QT_BEGIN("libdeflate");
  char in[4096]; for (int i = 0; i < 4096; i++) in[i] = "qnx"[i % 3];
  char comp[4096], out[4096]; size_t n;
  struct libdeflate_compressor *c = libdeflate_alloc_compressor(9);
  size_t cl = libdeflate_gzip_compress(c, in, sizeof in, comp, sizeof comp);
  QT_CHECK(cl > 0 && cl < 200, "compresses");
  struct libdeflate_decompressor *d = libdeflate_alloc_decompressor();
  QT_CHECK(libdeflate_gzip_decompress(d, comp, cl, out, sizeof out, &n) == LIBDEFLATE_SUCCESS && n == sizeof in, "decompresses");
  QT_CHECK(memcmp(in, out, sizeof in) == 0, "data intact");
  QT_CHECK(libdeflate_crc32(0, "123456789", 9) == 0xCBF43926u, "crc32");
  libdeflate_free_compressor(c); libdeflate_free_decompressor(d);
  QT_END();
}
