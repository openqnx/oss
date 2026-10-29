/* brotli: round-trip through the one-shot encoder/decoder. */
#include "common.h"
#include <stdlib.h>
#include <brotli/encode.h>
#include <brotli/decode.h>
int main(void) {
  QT_BEGIN("brotli");
  uint8_t in[8192];
  for (size_t i = 0; i < sizeof in; ++i) in[i] = (uint8_t)('x' + (i % 5));
  size_t clen = BrotliEncoderMaxCompressedSize(sizeof in);
  uint8_t *comp = malloc(clen), *out = malloc(sizeof in);
  QT_CHECK(comp && out, "allocate work buffers");
  BROTLI_BOOL ok = BrotliEncoderCompress(BROTLI_DEFAULT_QUALITY,
      BROTLI_DEFAULT_WINDOW, BROTLI_MODE_GENERIC, sizeof in, in, &clen, comp);
  QT_CHECK(ok == BROTLI_TRUE, "BrotliEncoderCompress");
  QT_CHECK(clen < sizeof in, "compressed output is smaller");
  size_t olen = sizeof in;
  BrotliDecoderResult res = BrotliDecoderDecompress(clen, comp, &olen, out);
  QT_CHECK(res == BROTLI_DECODER_RESULT_SUCCESS, "BrotliDecoderDecompress");
  QT_CHECK(olen == sizeof in && memcmp(in, out, sizeof in) == 0,
           "round trip is byte-identical");
  free(comp); free(out);
  QT_END();
}
