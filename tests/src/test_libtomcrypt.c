/* libtomcrypt with libtommath as its math library: known-answer vectors
 * (SHA-256 FIPS 180-2, AES-128 FIPS-197 appendix C.1, HMAC-SHA-256
 * RFC 4231 case 2, Base64 RFC 4648), the library's own self-tests, and an
 * RSA key generated, used to sign and verify through ltm_desc. */
#include <tomcrypt.h>
#include "common.h"

static int hexeq(const unsigned char *b, unsigned long n, const char *hex)
{
  char s[3];
  unsigned long i;
  if (strlen(hex) != 2 * n) return 0;
  for (i = 0; i < n; i++) { snprintf(s, sizeof s, "%02x", b[i]); if (memcmp(s, hex + 2 * i, 2)) return 0; }
  return 1;
}

int main(void)
{
  unsigned char out[64], key[16], pt[16], ct[16];
  unsigned long len, i;
  hash_state md;
  symmetric_key skey;
  QT_BEGIN("libtomcrypt");

  QT_CHECK(register_hash(&sha256_desc) >= 0 && register_cipher(&aes_desc) >= 0
           && register_prng(&sprng_desc) >= 0, "register sha256, aes, sprng");

  sha256_init(&md); sha256_process(&md, (const unsigned char *)"abc", 3); sha256_done(&md, out);
  QT_CHECK(hexeq(out, 32, "ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad"), "SHA-256(\"abc\") (FIPS 180-2)");

  for (i = 0; i < 16; i++) { key[i] = (unsigned char)i; pt[i] = (unsigned char)(i * 0x11); }
  QT_CHECK(aes_setup(key, 16, 0, &skey) == CRYPT_OK && aes_ecb_encrypt(pt, ct, &skey) == CRYPT_OK
           && hexeq(ct, 16, "69c4e0d86a7b0430d8cdb78070b4c55a"), "AES-128 FIPS-197 C.1 encrypt");
  QT_CHECK(aes_ecb_decrypt(ct, out, &skey) == CRYPT_OK && memcmp(out, pt, 16) == 0, "AES-128 decrypt round trip");

  len = sizeof out;
  QT_CHECK(hmac_memory(find_hash("sha256"), (const unsigned char *)"Jefe", 4,
                       (const unsigned char *)"what do ya want for nothing?", 28, out, &len) == CRYPT_OK
           && hexeq(out, len, "5bdcc146bf60754e6a042426089575c75a003f089d2739839dec58b964ec3843"), "HMAC-SHA-256 (RFC 4231 case 2)");

  len = sizeof out;
  QT_CHECK(base64_encode((const unsigned char *)"foobar", 6, out, &len) == CRYPT_OK && len == 8
           && memcmp(out, "Zm9vYmFy", 8) == 0, "Base64(\"foobar\") = Zm9vYmFy (RFC 4648)");

  QT_CHECK(sha256_test() == CRYPT_OK, "sha256_test() self-test");
  QT_CHECK(rijndael_test() == CRYPT_OK, "AES self-test");
  QT_CHECK(gcm_test() == CRYPT_OK, "GCM self-test");
  QT_CHECK(chacha20poly1305_test() == CRYPT_OK, "ChaCha20-Poly1305 self-test");

  /* Public key through the libtommath descriptor. */
  ltc_mp = ltm_desc;
  {
    rsa_key rk;
    prng_state prng;
    unsigned char hash[32], sig[256];
    unsigned long siglen = sizeof sig;
    int stat = 0, pi = find_prng("sprng"), hi = find_hash("sha256");
    QT_CHECK(rng_make_prng(128, pi, &prng, NULL) == CRYPT_OK, "PRNG seeded from the system (/dev/urandom)");
    QT_CHECK(rsa_make_key(&prng, pi, 1024 / 8, 65537, &rk) == CRYPT_OK, "RSA-1024 key generated (ltm_desc)");
    sha256_init(&md); sha256_process(&md, (const unsigned char *)"QNX", 3); sha256_done(&md, hash);
    QT_CHECK(rsa_sign_hash_ex(hash, 32, sig, &siglen, LTC_PKCS_1_V1_5, &prng, pi, hi, 0, &rk) == CRYPT_OK && siglen == 128,
             "RSA PKCS#1 v1.5 signature, 128 bytes");
    QT_CHECK(rsa_verify_hash_ex(sig, siglen, hash, 32, LTC_PKCS_1_V1_5, hi, 0, &stat, &rk) == CRYPT_OK && stat == 1,
             "signature verifies");
    hash[0] ^= 1; stat = 1;
    QT_CHECK(rsa_verify_hash_ex(sig, siglen, hash, 32, LTC_PKCS_1_V1_5, hi, 0, &stat, &rk) == CRYPT_OK && stat == 0,
             "a changed hash does not verify");
    rsa_free(&rk);
  }
  QT_END();
}
