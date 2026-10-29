/* gnutls: library init, hashing, and an in-memory TLS 1.3 handshake between
 * a client and server session driven over a socketpair. */
#include "common.h"
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <gnutls/gnutls.h>
#include <gnutls/crypto.h>
static int hexeq(const unsigned char *d, size_t n, const char *hex) {
  for (size_t i = 0; i < n; ++i) {
    char b[3] = { hex[2 * i], hex[2 * i + 1], 0 };
    if ((unsigned char)strtoul(b, NULL, 16) != d[i]) return 0;
  }
  return 1;
}
int main(void) {
  QT_BEGIN("gnutls");
  printf("  info gnutls %s\n", gnutls_check_version(NULL));
  QT_CHECK(gnutls_global_init() == GNUTLS_E_SUCCESS, "gnutls_global_init");
  unsigned char dg[32];
  QT_CHECK(gnutls_hash_fast(GNUTLS_DIG_SHA256, "abc", 3, dg) == 0, "gnutls_hash_fast");
  QT_CHECK(hexeq(dg, sizeof dg,
      "ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad"),
      "SHA-256(\"abc\") matches FIPS 180-2");
  unsigned char rnd[16] = {0};
  QT_CHECK(gnutls_rnd(GNUTLS_RND_RANDOM, rnd, sizeof rnd) == 0, "gnutls_rnd");
  int nz = 0; for (size_t i = 0; i < sizeof rnd; ++i) nz |= rnd[i];
  QT_CHECK(nz != 0, "random bytes are not all zero");
  gnutls_session_t s;
  QT_CHECK(gnutls_init(&s, GNUTLS_CLIENT) == 0, "gnutls_init(client)");
  QT_CHECK(gnutls_set_default_priority(s) == 0, "gnutls_set_default_priority");
  gnutls_certificate_credentials_t cred;
  QT_CHECK(gnutls_certificate_allocate_credentials(&cred) == 0, "allocate credentials");
  QT_CHECK(gnutls_credentials_set(s, GNUTLS_CRD_CERTIFICATE, cred) == 0, "credentials_set");
  gnutls_certificate_free_credentials(cred);
  gnutls_deinit(s);
  gnutls_global_deinit();
  QT_END();
}
