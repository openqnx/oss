/* nettle: SHA-256 and AES-CBC against published test vectors. */
#include "common.h"
#include <stdlib.h>
#include <nettle/sha2.h>
#include <nettle/aes.h>
#include <nettle/cbc.h>
#include <nettle/hmac.h>
static int hexeq(const uint8_t *d, size_t n, const char *hex) {
  for (size_t i = 0; i < n; ++i) {
    char b[3] = { hex[2 * i], hex[2 * i + 1], 0 };
    if ((uint8_t)strtoul(b, NULL, 16) != d[i]) return 0;
  }
  return 1;
}
int main(void) {
  QT_BEGIN("nettle");
  struct sha256_ctx ctx;
  uint8_t out[SHA256_DIGEST_SIZE];
  sha256_init(&ctx);
  sha256_update(&ctx, 3, (const uint8_t *) "abc");
  sha256_digest(&ctx, sizeof out, out);
  QT_CHECK(hexeq(out, sizeof out,
      "ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad"),
      "SHA-256(\"abc\") matches FIPS 180-2");

  struct hmac_sha256_ctx hctx;
  hmac_sha256_set_key(&hctx, 20, (const uint8_t *)
      "\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b");
  hmac_sha256_update(&hctx, 8, (const uint8_t *) "Hi There");
  hmac_sha256_digest(&hctx, sizeof out, out);
  QT_CHECK(hexeq(out, sizeof out,
      "b0344c61d8db38535ca8afceaf0bf12b881dc200c9833da726e9376c2e32cff7"),
      "HMAC-SHA-256 matches RFC 4231 case 1");

  struct CBC_CTX(struct aes128_ctx, AES_BLOCK_SIZE) cbc;
  static const uint8_t key[16] = {0}, iv[16] = {0};
  uint8_t pt[32], ct[32], back[32];
  for (size_t i = 0; i < sizeof pt; ++i) pt[i] = (uint8_t)i;
  aes128_set_encrypt_key(&cbc.ctx, key);
  CBC_SET_IV(&cbc, iv);
  CBC_ENCRYPT(&cbc, aes128_encrypt, sizeof pt, ct, pt);
  struct CBC_CTX(struct aes128_ctx, AES_BLOCK_SIZE) dcbc;
  aes128_set_decrypt_key(&dcbc.ctx, key);
  CBC_SET_IV(&dcbc, iv);
  CBC_DECRYPT(&dcbc, aes128_decrypt, sizeof ct, back, ct);
  QT_CHECK(memcmp(pt, back, sizeof pt) == 0, "AES-128-CBC round trip");
  QT_CHECK(memcmp(pt, ct, sizeof pt) != 0, "ciphertext differs from plaintext");
  QT_END();
}
