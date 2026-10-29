#include "common.h"
#include <zlib.h>
int main(void) {
  QT_BEGIN("zlib");
  const char src[] = "QNX SDP 8.0 zlib round trip -- aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa";
  uLong slen = (uLong)sizeof(src);
  uLong clen = compressBound(slen);
  Bytef cbuf[512], dbuf[512];
  QT_CHECK(compress(cbuf, &clen, (const Bytef *)src, slen) == Z_OK, "compress");
  QT_CHECK(clen < slen, "compressed smaller than input");
  uLong dlen = sizeof(dbuf);
  QT_CHECK(uncompress(dbuf, &dlen, cbuf, clen) == Z_OK, "uncompress");
  QT_CHECK(dlen == slen && memcmp(dbuf, src, slen) == 0, "round trip matches");
  printf("  info zlib %s\n", zlibVersion());
  QT_END();
}
