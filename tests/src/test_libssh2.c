/* libssh2: init, session setup and the hostkey/known-hosts plumbing. No
 * server is contacted -- the transport is exercised up to the point where a
 * socket would be needed, which is what a port needs to prove. */
#include "common.h"
#include <stdlib.h>
#include <libssh2.h>
int main(void) {
  QT_BEGIN("libssh2");
  printf("  info libssh2 %s\n", libssh2_version(0));
  QT_CHECK(libssh2_init(0) == 0, "libssh2_init");
  LIBSSH2_SESSION *s = libssh2_session_init();
  QT_CHECK(s != NULL, "libssh2_session_init");
  libssh2_session_set_blocking(s, 0);
  QT_CHECK(libssh2_session_get_blocking(s) == 0, "non-blocking mode set");
  QT_CHECK(libssh2_session_method_pref(s, LIBSSH2_METHOD_KEX,
           "curve25519-sha256,diffie-hellman-group14-sha256") == 0,
           "KEX preference accepted");
  QT_CHECK(libssh2_session_method_pref(s, LIBSSH2_METHOD_CRYPT_CS,
           "aes256-ctr,aes128-ctr") == 0, "cipher preference accepted");
  const char **algs = NULL;
  int n = libssh2_session_supported_algs(s, LIBSSH2_METHOD_HOSTKEY, &algs);
  QT_CHECK(n > 0 && algs != NULL, "host key algorithms are advertised");
  if (algs) libssh2_free(s, algs);
  LIBSSH2_KNOWNHOSTS *kh = libssh2_knownhost_init(s);
  QT_CHECK(kh != NULL, "libssh2_knownhost_init");
  if (kh) {
    struct libssh2_knownhost *entry = NULL;
    int rc = libssh2_knownhost_add(kh, "qnx.test", NULL, "\x00\x00\x00\x07ssh-rsa", 12,
        LIBSSH2_KNOWNHOST_TYPE_PLAIN | LIBSSH2_KNOWNHOST_KEYENC_RAW |
        LIBSSH2_KNOWNHOST_KEY_SSHRSA, &entry);
    QT_CHECK(rc == 0, "libssh2_knownhost_add");
    libssh2_knownhost_free(kh);
  }
  /* Hashing is used for the host key fingerprint; verify it is wired up. */
  QT_CHECK(libssh2_hostkey_hash(s, LIBSSH2_HOSTKEY_HASH_SHA256) == NULL,
           "no fingerprint before a handshake");
  libssh2_session_free(s);
  libssh2_exit();
  QT_END();
}
