/* BoringSSL (its own prefix, /usr/local/opt/boringssl): known-answer checks
 * of the primitives -- SHA-256 ("abc", FIPS 180-2), HMAC-SHA-256 (RFC 4231
 * case 2), AES-128-GCM (the GCM spec's test case 2, plus a tampered tag
 * that must be refused), X25519 (RFC 7748 6.1), Ed25519 (RFC 8032 test 1)
 * -- the system entropy source, and a TLS 1.3 handshake between a client
 * and a server in this process over a socketpair, with a self-signed P-256
 * certificate the client verifies, then data both ways. */
#include <openssl/aead.h>
#include <openssl/bytestring.h>
#include <openssl/curve25519.h>
#include <openssl/digest.h>
#include <openssl/ec_key.h>
#include <openssl/err.h>
#include <openssl/evp.h>
#include <openssl/hmac.h>
#include <openssl/nid.h>
#include <openssl/rand.h>
#include <openssl/sha.h>
#include <openssl/ssl.h>
#include <openssl/x509.h>

#include <fcntl.h>
#include <stdint.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <unistd.h>

#include "../common.h"

static int unhex(const char *h, uint8_t *out, size_t max) {
  size_t n = strlen(h) / 2;
  if (n > max) return -1;
  for (size_t i = 0; i < n; i++) {
    unsigned v;
    if (sscanf(h + 2 * i, "%2x", &v) != 1) return -1;
    out[i] = (uint8_t)v;
  }
  return (int)n;
}

static int eqhex(const uint8_t *b, size_t n, const char *h) {
  uint8_t want[128];
  int wn = unhex(h, want, sizeof(want));
  return wn == (int)n && memcmp(b, want, n) == 0;
}

static int pump(SSL *c, SSL *s) {
  /* Drive both ends of a non-blocking handshake until both are done. */
  int cdone = 0, sdone = 0;
  for (int i = 0; i < 200 && !(cdone && sdone); i++) {
    if (!cdone) {
      int r = SSL_do_handshake(c);
      if (r == 1) cdone = 1;
      else {
        int e = SSL_get_error(c, r);
        if (e != SSL_ERROR_WANT_READ && e != SSL_ERROR_WANT_WRITE) {
          printf("  info client handshake error %d: %s\n", e,
                 ERR_error_string(ERR_get_error(), NULL));
          return 0;
        }
      }
    }
    if (!sdone) {
      int r = SSL_do_handshake(s);
      if (r == 1) sdone = 1;
      else {
        int e = SSL_get_error(s, r);
        if (e != SSL_ERROR_WANT_READ && e != SSL_ERROR_WANT_WRITE) {
          printf("  info server handshake error %d: %s\n", e,
                 ERR_error_string(ERR_get_error(), NULL));
          return 0;
        }
      }
    }
  }
  return cdone && sdone;
}

static int read_retry(SSL *ssl, char *buf, int len) {
  for (int i = 0; i < 200; i++) {
    int r = SSL_read(ssl, buf, len);
    if (r > 0) return r;
    if (SSL_get_error(ssl, r) != SSL_ERROR_WANT_READ) return r;
    usleep(1000);
  }
  return -1;
}

int main(void) {
  QT_BEGIN("boringssl");
#ifdef OPENSSL_IS_BORINGSSL
  QT_CHECK(1, "headers are BoringSSL's (OPENSSL_IS_BORINGSSL)");
#else
  QT_CHECK(0, "headers are BoringSSL's (OPENSSL_IS_BORINGSSL)");
#endif
  printf("  info %s\n", OpenSSL_version(OPENSSL_VERSION));

  /* SHA-256 */
  uint8_t md[SHA256_DIGEST_LENGTH];
  SHA256((const uint8_t *)"abc", 3, md);
  QT_CHECK(eqhex(md, sizeof(md),
                 "ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad"),
           "SHA-256(\"abc\")");

  /* HMAC-SHA-256, RFC 4231 test case 2 */
  unsigned mdlen = 0;
  const char *data = "what do ya want for nothing?";
  HMAC(EVP_sha256(), "Jefe", 4, (const uint8_t *)data, strlen(data), md, &mdlen);
  QT_CHECK(mdlen == 32 &&
               eqhex(md, mdlen,
                     "5bdcc146bf60754e6a042426089575c75a003f089d2739839dec58b964ec3843"),
           "HMAC-SHA-256 (RFC 4231 case 2)");

  /* AES-128-GCM: zero key, zero 96-bit IV, one zero block */
  {
    uint8_t key[16] = {0}, iv[12] = {0}, pt[16] = {0}, out[32], back[16];
    size_t outlen = 0, backlen = 0;
    EVP_AEAD_CTX *ctx = EVP_AEAD_CTX_new(EVP_aead_aes_128_gcm(), key, sizeof(key),
                                         EVP_AEAD_DEFAULT_TAG_LENGTH);
    int ok = ctx && EVP_AEAD_CTX_seal(ctx, out, &outlen, sizeof(out), iv, sizeof(iv),
                                      pt, sizeof(pt), NULL, 0);
    QT_CHECK(ok && outlen == 32 &&
                 eqhex(out, 32, "0388dace60b6a392f328c2b971b2fe78"
                                "ab6e47d42cec13bdf53a67b21257bddf"),
             "AES-128-GCM seal: ciphertext and tag (GCM test case 2)");
    ok = ctx && EVP_AEAD_CTX_open(ctx, back, &backlen, sizeof(back), iv, sizeof(iv),
                                  out, outlen, NULL, 0);
    QT_CHECK(ok && backlen == 16 && memcmp(back, pt, 16) == 0, "AES-128-GCM open");
    out[31] ^= 1;
    ok = ctx && EVP_AEAD_CTX_open(ctx, back, &backlen, sizeof(back), iv, sizeof(iv),
                                  out, outlen, NULL, 0);
    QT_CHECK(!ok, "AES-128-GCM open refuses a tampered tag");
    ERR_clear_error();
    EVP_AEAD_CTX_free(ctx);
  }

  /* X25519, RFC 7748 section 6.1 */
  {
    uint8_t a[32], bpub[32], apub[32], shared[32];
    unhex("77076d0a7318a57d3c16c17251b26645df4c2f87ebc0992ab177fba51db92c2a", a, 32);
    unhex("de9edb7d7b7dc1b4d35b61c2ece435373f8343c85b78674dadfc7e146f882b4f", bpub, 32);
    X25519_public_from_private(apub, a);
    QT_CHECK(eqhex(apub, 32, "8520f0098930a754748b7ddcb43ef75a0dbf3a0d26381af4eba4a98eaa9b4e6a"),
             "X25519 public key (RFC 7748 6.1, Alice)");
    QT_CHECK(X25519(shared, a, bpub) &&
                 eqhex(shared, 32,
                       "4a5d9d5ba4ce2de1728e3bf480350f25e07e21c947d19e3376f09b3c1e161742"),
             "X25519 shared secret (RFC 7748 6.1)");
  }

  /* Ed25519, RFC 8032 test 1 (empty message) */
  {
    uint8_t seed[32], pub[32], priv[64], sig[64];
    unhex("9d61b19deffd5a60ba844af492ec2cc44449c5697b326919703bac031cae7f60", seed, 32);
    ED25519_keypair_from_seed(pub, priv, seed);
    QT_CHECK(eqhex(pub, 32, "d75a980182b10ab7d54bfed3c964073a0ee172f3daa62325af021a68f707511a"),
             "Ed25519 public key (RFC 8032 test 1)");
    QT_CHECK(ED25519_sign(sig, (const uint8_t *)"", 0, priv) &&
                 eqhex(sig, 64,
                       "e5564300c360ac729086e2cc806e828a84877f1eb8e5d974d873e065224901555fb8821590a33bacc61e39701cf9b46bd25bf5f0595bbe24655141438e7a100b"),
             "Ed25519 signature (RFC 8032 test 1)");
    QT_CHECK(ED25519_verify((const uint8_t *)"", 0, sig, pub), "Ed25519 verify");
    sig[0] ^= 1;
    QT_CHECK(!ED25519_verify((const uint8_t *)"", 0, sig, pub),
             "Ed25519 verify refuses a modified signature");
  }

  /* Entropy: two 32-byte draws, not all zero, not equal */
  {
    uint8_t r1[32] = {0}, r2[32] = {0}, z[32] = {0};
    int ok = RAND_bytes(r1, 32) && RAND_bytes(r2, 32);
    QT_CHECK(ok && memcmp(r1, z, 32) != 0 && memcmp(r1, r2, 32) != 0,
             "RAND_bytes: two different non-zero draws");
  }

  /* TLS 1.3 over a socketpair, self-signed P-256 certificate */
  {
    EVP_PKEY *pkey = EVP_PKEY_new();
    EC_KEY *ec = EC_KEY_new_by_curve_name(NID_X9_62_prime256v1);
    int ok = pkey && ec && EC_KEY_generate_key(ec) && EVP_PKEY_assign_EC_KEY(pkey, ec);
    X509 *x = X509_new();
    ok = ok && x && X509_set_version(x, X509_VERSION_3) &&
         ASN1_INTEGER_set(X509_get_serialNumber(x), 1) &&
         X509_gmtime_adj(X509_getm_notBefore(x), -60) &&
         X509_gmtime_adj(X509_getm_notAfter(x), 3600);
    X509_NAME *name = ok ? X509_get_subject_name(x) : NULL;
    ok = ok && X509_NAME_add_entry_by_txt(name, "CN", MBSTRING_ASC,
                                          (const uint8_t *)"localhost", -1, -1, 0) &&
         X509_set_issuer_name(x, name) && X509_set_pubkey(x, pkey) &&
         X509_sign(x, pkey, EVP_sha256());
    QT_CHECK(ok, "self-signed P-256 certificate created");

    SSL_CTX *sctx = SSL_CTX_new(TLS_method()), *cctx = SSL_CTX_new(TLS_method());
    ok = ok && sctx && cctx && SSL_CTX_use_certificate(sctx, x) &&
         SSL_CTX_use_PrivateKey(sctx, pkey) &&
         SSL_CTX_set_min_proto_version(cctx, TLS1_3_VERSION) &&
         X509_STORE_add_cert(SSL_CTX_get_cert_store(cctx), x);
    if (cctx) SSL_CTX_set_verify(cctx, SSL_VERIFY_PEER, NULL);
    int sv[2] = {-1, -1};
    ok = ok && socketpair(AF_UNIX, SOCK_STREAM, 0, sv) == 0;
    QT_CHECK(ok, "contexts and socketpair set up");
    SSL *c = NULL, *s = NULL;
    if (ok) {
      fcntl(sv[0], F_SETFL, O_NONBLOCK);
      fcntl(sv[1], F_SETFL, O_NONBLOCK);
      c = SSL_new(cctx);
      s = SSL_new(sctx);
      SSL_set_fd(c, sv[0]);
      SSL_set_fd(s, sv[1]);
      SSL_set_connect_state(c);
      SSL_set_accept_state(s);
      SSL_set_tlsext_host_name(c, "localhost");
      ok = pump(c, s);
      QT_CHECK(ok, "TLS handshake completed");
      QT_CHECK(ok && SSL_version(c) == TLS1_3_VERSION, "negotiated TLS 1.3");
      QT_CHECK(ok && SSL_get_verify_result(c) == X509_V_OK,
               "client verified the server certificate against its store");
      const SSL_CIPHER *ci = ok ? SSL_get_current_cipher(c) : NULL;
      printf("  info cipher %s, group %s\n", ci ? SSL_CIPHER_get_name(ci) : "?",
             ok ? SSL_get_group_name(SSL_get_group_id(c)) : "?");
      const char *sni = ok ? SSL_get_servername(s, TLSEXT_NAMETYPE_host_name) : NULL;
      QT_CHECK(sni && strcmp(sni, "localhost") == 0, "server saw SNI \"localhost\"");
      char buf[64];
      int n = ok ? SSL_write(c, "ping from client", 16) : -1;
      int r = n == 16 ? read_retry(s, buf, sizeof(buf)) : -1;
      QT_CHECK(r == 16 && memcmp(buf, "ping from client", 16) == 0,
               "client -> server application data");
      n = ok ? SSL_write(s, "pong from server", 16) : -1;
      r = n == 16 ? read_retry(c, buf, sizeof(buf)) : -1;
      QT_CHECK(r == 16 && memcmp(buf, "pong from server", 16) == 0,
               "server -> client application data");
    }
    SSL_free(c);
    SSL_free(s);
    SSL_CTX_free(cctx);
    SSL_CTX_free(sctx);
    X509_free(x);
    EVP_PKEY_free(pkey);
    if (sv[0] >= 0) close(sv[0]);
    if (sv[1] >= 0) close(sv[1]);
  }
  QT_END();
}
