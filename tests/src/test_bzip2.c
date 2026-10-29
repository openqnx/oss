/* bzip2: round-trip through the buffer-to-buffer API. */
#include "common.h"
#include <stdlib.h>
#include <bzlib.h>
int main(void) {
  QT_BEGIN("bzip2");
  QT_CHECK(BZ2_bzlibVersion() != NULL, "version string");
  char in[16384];
  for (size_t i = 0; i < sizeof in; ++i) in[i] = (char)('A' + (i % 23));
  unsigned clen = sizeof in + sizeof in / 100 + 600, dlen = sizeof in;
  char *comp = malloc(clen), *out = malloc(dlen);
  QT_CHECK(comp && out, "allocate work buffers");
  int rc = BZ2_bzBuffToBuffCompress(comp, &clen, in, sizeof in, 9, 0, 30);
  QT_CHECK(rc == BZ_OK, "BZ2_bzBuffToBuffCompress");
  QT_CHECK(clen < sizeof in, "compressed output is smaller");
  rc = BZ2_bzBuffToBuffDecompress(out, &dlen, comp, clen, 0, 0);
  QT_CHECK(rc == BZ_OK, "BZ2_bzBuffToBuffDecompress");
  QT_CHECK(dlen == sizeof in && memcmp(in, out, sizeof in) == 0,
           "round trip is byte-identical");
  free(comp); free(out);
  QT_END();
}
