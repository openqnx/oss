/* zstd: round-trip a buffer through the simple one-shot API. */
#include "common.h"
#include <stdlib.h>
#include <zstd.h>
int main(void) {
  QT_BEGIN("zstd");
  QT_CHECK(ZSTD_versionNumber() >= 10500, "version >= 1.5");
  char in[4096];
  for (size_t i = 0; i < sizeof in; ++i) in[i] = (char)('a' + (i % 17));
  size_t bound = ZSTD_compressBound(sizeof in);
  void *comp = malloc(bound), *out = malloc(sizeof in);
  QT_CHECK(comp && out, "allocate work buffers");
  size_t csz = ZSTD_compress(comp, bound, in, sizeof in, 9);
  QT_CHECK(!ZSTD_isError(csz), "compress succeeds");
  QT_CHECK(csz < sizeof in, "output is smaller than input");
  size_t dsz = ZSTD_decompress(out, sizeof in, comp, csz);
  QT_CHECK(dsz == sizeof in, "decompressed size matches");
  QT_CHECK(memcmp(in, out, sizeof in) == 0, "round trip is byte-identical");
  free(comp); free(out);
  QT_END();
}
