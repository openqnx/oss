#include "common.h"
#include <sodium.h>
int main(void) {
  QT_BEGIN("libsodium");
  QT_CHECK(sodium_init() >= 0, "sodium_init");
  unsigned char key[crypto_secretbox_KEYBYTES], nonce[crypto_secretbox_NONCEBYTES];
  crypto_secretbox_keygen(key); randombytes_buf(nonce, sizeof(nonce));
  const unsigned char msg[] = "secure on-board comms";
  unsigned char ct[crypto_secretbox_MACBYTES + sizeof(msg)];
  unsigned char pt[sizeof(msg)];
  QT_CHECK(crypto_secretbox_easy(ct, msg, sizeof(msg), nonce, key) == 0, "secretbox seal");
  QT_CHECK(crypto_secretbox_open_easy(pt, ct, sizeof(ct), nonce, key) == 0, "secretbox open");
  QT_CHECK(memcmp(pt, msg, sizeof(msg)) == 0, "round trip matches");
  printf("  info libsodium %s\n", sodium_version_string());
  QT_END();
}
