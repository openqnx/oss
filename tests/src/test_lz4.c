#include "common.h"
#include <lz4.h>
int main(void) {
  QT_BEGIN("lz4");
  const char src[] = "QNX lz4 round trip payload -- bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb";
  char cbuf[512], dbuf[512];
  int n = LZ4_compress_default(src, cbuf, (int)sizeof(src), (int)sizeof(cbuf));
  QT_CHECK(n > 0, "LZ4_compress_default");
  int d = LZ4_decompress_safe(cbuf, dbuf, n, (int)sizeof(dbuf));
  QT_CHECK(d == (int)sizeof(src), "decompressed size matches");
  QT_CHECK(memcmp(dbuf, src, sizeof(src)) == 0, "round trip matches");
  printf("  info lz4 %s\n", LZ4_versionString());
  QT_END();
}
