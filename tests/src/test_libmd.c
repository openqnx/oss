#include "common.h"
#include <string.h>
#include <md5.h>
#include <sha2.h>
#include <rmd160.h>
/* libmd's BSD digest API against the published test vectors for "abc". */
int main(void) {
  QT_BEGIN("libmd");
  char out[129];
  QT_CHECK(strcmp(MD5Data((const unsigned char *)"abc", 3, out), "900150983cd24fb0d6963f7d28e17f72") == 0, "MD5(abc)");
  QT_CHECK(strcmp(SHA256Data((const unsigned char *)"abc", 3, out),
           "ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad") == 0, "SHA256(abc)");
  QT_CHECK(strcmp(RMD160Data((const unsigned char *)"abc", 3, out), "8eb208f7e05d987a9b044a8e98c6b087f15a0bfc") == 0, "RIPEMD160(abc)");
  SHA2_CTX c; SHA512Init(&c); SHA512Update(&c, (const unsigned char *)"a", 1); SHA512Update(&c, (const unsigned char *)"bc", 2);
  SHA512End(&c, out);
  QT_CHECK(strncmp(out, "ddaf35a193617aba", 16) == 0, "SHA512(abc) in two updates");
  QT_END();
}
