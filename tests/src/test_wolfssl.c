#include "common.h"
#include <wolfssl/options.h>
#include <wolfssl/wolfcrypt/sha256.h>
#include <wolfssl/version.h>
int main(void) {
  QT_BEGIN("wolfssl");
  wc_Sha256 sha; unsigned char out[32];
  QT_CHECK(wc_InitSha256(&sha) == 0, "wc_InitSha256");
  QT_CHECK(wc_Sha256Update(&sha, (const byte *)"abc", 3) == 0, "wc_Sha256Update");
  QT_CHECK(wc_Sha256Final(&sha, out) == 0, "wc_Sha256Final");
  QT_CHECK(out[0] == 0xba && out[1] == 0x78 && out[2] == 0x16 && out[3] == 0xbf,
           "sha256(\"abc\") matches known vector");
  wc_Sha256Free(&sha);
  printf("  info wolfssl %s\n", LIBWOLFSSL_VERSION_STRING);
  QT_END();
}
