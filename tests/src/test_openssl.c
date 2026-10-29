#include "common.h"
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <openssl/opensslv.h>
int main(void) {
  QT_BEGIN("openssl");
  unsigned char md[EVP_MAX_MD_SIZE]; unsigned int mdlen = 0;
  EVP_MD_CTX *c = EVP_MD_CTX_new();
  QT_CHECK(c != NULL, "EVP_MD_CTX_new");
  QT_CHECK(EVP_DigestInit_ex(c, EVP_sha256(), NULL) == 1, "sha256 init");
  QT_CHECK(EVP_DigestUpdate(c, "abc", 3) == 1, "sha256 update");
  QT_CHECK(EVP_DigestFinal_ex(c, md, &mdlen) == 1, "sha256 final");
  /* FIPS 180-4 SHA-256("abc") starts ba7816bf */
  QT_CHECK(mdlen == 32 && md[0] == 0xba && md[1] == 0x78 && md[2] == 0x16 && md[3] == 0xbf,
           "sha256(\"abc\") matches known vector");
  EVP_MD_CTX_free(c);
  unsigned char r[16] = {0};
  QT_CHECK(RAND_bytes(r, sizeof(r)) == 1, "RAND_bytes");
  printf("  info %s\n", OPENSSL_VERSION_TEXT);
  QT_END();
}
