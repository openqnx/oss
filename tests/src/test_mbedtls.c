#include "common.h"
#include <mbedtls/sha256.h>
#include <mbedtls/version.h>
int main(void) {
  QT_BEGIN("mbedtls");
  unsigned char out[32];
  QT_CHECK(mbedtls_sha256((const unsigned char *)"abc", 3, out, 0) == 0, "mbedtls_sha256");
  QT_CHECK(out[0] == 0xba && out[1] == 0x78 && out[2] == 0x16 && out[3] == 0xbf,
           "sha256(\"abc\") matches known vector");
  char v[32] = {0}; mbedtls_version_get_string(v);
  printf("  info mbedtls %s\n", v);
  QT_END();
}
