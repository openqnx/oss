/* xz/liblzma: round-trip through the .xz container using the easy API. */
#include "common.h"
#include <stdlib.h>
#include <lzma.h>
int main(void) {
  QT_BEGIN("xz (liblzma)");
  uint8_t in[8192];
  for (size_t i = 0; i < sizeof in; ++i) in[i] = (uint8_t)(i * 7);
  size_t bound = lzma_stream_buffer_bound(sizeof in);
  uint8_t *comp = malloc(bound), *out = malloc(sizeof in);
  QT_CHECK(comp && out, "allocate work buffers");
  size_t cpos = 0;
  lzma_ret r = lzma_easy_buffer_encode(6, LZMA_CHECK_CRC64, NULL,
                                       in, sizeof in, comp, &cpos, bound);
  QT_CHECK(r == LZMA_OK, "lzma_easy_buffer_encode");
  QT_CHECK(cpos > 0 && cpos < sizeof in, "compressed output is smaller");
  size_t ipos = 0, opos = 0;
  uint64_t memlimit = 64u << 20;
  r = lzma_stream_buffer_decode(&memlimit, 0, NULL, comp, &ipos, cpos,
                                out, &opos, sizeof in);
  QT_CHECK(r == LZMA_OK, "lzma_stream_buffer_decode");
  QT_CHECK(opos == sizeof in && memcmp(in, out, sizeof in) == 0,
           "round trip is byte-identical");
  free(comp); free(out);
  QT_END();
}
