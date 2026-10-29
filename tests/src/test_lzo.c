/* LZO: LZO1X-1 compress and safe decompress. */
#include "common.h"
#include <lzo/lzo1x.h>
static unsigned char wrk[LZO1X_1_MEM_COMPRESS];
int main(void) {
  QT_BEGIN("lzo");
  QT_CHECK(lzo_init() == LZO_E_OK, "lzo_init");
  unsigned char in[8192], c[8192 + 8192 / 16 + 67], out[8192]; lzo_uint cl, ol = sizeof out;
  for (int i = 0; i < 8192; i++) in[i] = (unsigned char)(i % 7);
  QT_CHECK(lzo1x_1_compress(in, sizeof in, c, &cl, wrk) == LZO_E_OK && cl < sizeof in, "compresses");
  QT_CHECK(lzo1x_decompress_safe(c, cl, out, &ol, NULL) == LZO_E_OK && ol == sizeof in, "decompresses");
  QT_CHECK(memcmp(in, out, sizeof in) == 0, "data intact");
  QT_END();
}
