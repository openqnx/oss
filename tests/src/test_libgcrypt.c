/* libgcrypt: init (which runs its self-tests), hash and cipher vectors. */
#include "common.h"
#include <stdlib.h>
#include <gcrypt.h>
static int hexeq(const unsigned char *d, size_t n, const char *hex) {
  for (size_t i = 0; i < n; ++i) {
    char b[3] = { hex[2 * i], hex[2 * i + 1], 0 };
    if ((unsigned char)strtoul(b, NULL, 16) != d[i]) return 0;
  }
  return 1;
}
int main(void) {
  QT_BEGIN("libgcrypt");
  const char *ver = gcry_check_version(GCRYPT_VERSION);
  QT_CHECK(ver != NULL, "gcry_check_version matches the headers");
  printf("  info libgcrypt %s\n", ver ? ver : "(null)");
  gcry_control(GCRYCTL_DISABLE_SECMEM, 0);
  gcry_control(GCRYCTL_INITIALIZATION_FINISHED, 0);
  QT_CHECK(gcry_control(GCRYCTL_INITIALIZATION_FINISHED_P) != 0, "initialisation finished");
  unsigned char dg[32];
  gcry_md_hash_buffer(GCRY_MD_SHA256, dg, "abc", 3);
  QT_CHECK(hexeq(dg, sizeof dg,
      "ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad"),
      "SHA-256(\"abc\") matches FIPS 180-2");
  gcry_cipher_hd_t hd;
  QT_CHECK(gcry_cipher_open(&hd, GCRY_CIPHER_AES128, GCRY_CIPHER_MODE_CBC, 0) == 0,
           "gcry_cipher_open AES-128-CBC");
  unsigned char key[16] = {0}, iv[16] = {0}, pt[32], ct[32], back[32];
  for (size_t i = 0; i < sizeof pt; ++i) pt[i] = (unsigned char)i;
  gcry_cipher_setkey(hd, key, sizeof key);
  gcry_cipher_setiv(hd, iv, sizeof iv);
  QT_CHECK(gcry_cipher_encrypt(hd, ct, sizeof ct, pt, sizeof pt) == 0, "encrypt");
  gcry_cipher_close(hd);
  gcry_cipher_open(&hd, GCRY_CIPHER_AES128, GCRY_CIPHER_MODE_CBC, 0);
  gcry_cipher_setkey(hd, key, sizeof key);
  gcry_cipher_setiv(hd, iv, sizeof iv);
  QT_CHECK(gcry_cipher_decrypt(hd, back, sizeof back, ct, sizeof ct) == 0, "decrypt");
  QT_CHECK(memcmp(pt, back, sizeof pt) == 0, "AES-128-CBC round trip");
  gcry_cipher_close(hd);
  unsigned char r1[8] = {0}, r2[8] = {0};
  gcry_randomize(r1, sizeof r1, GCRY_STRONG_RANDOM);
  gcry_randomize(r2, sizeof r2, GCRY_STRONG_RANDOM);
  QT_CHECK(memcmp(r1, r2, sizeof r1) != 0, "two random draws differ");
  QT_END();
}
