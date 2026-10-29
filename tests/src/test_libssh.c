/* libssh: generate a key pair, export and re-import the public key. */
#include "common.h"
#include <libssh/libssh.h>
#include <stdlib.h>
int main(void) {
  QT_BEGIN("libssh");
  QT_CHECK(ssh_init() == 0, "init");
  ssh_key k, pub; char *b64 = NULL;
  QT_CHECK(ssh_pki_generate(SSH_KEYTYPE_ED25519, 0, &k) == SSH_OK, "ed25519 keygen");
  QT_CHECK(ssh_pki_export_privkey_to_pubkey(k, &pub) == SSH_OK, "derive public key");
  QT_CHECK(ssh_pki_export_pubkey_base64(pub, &b64) == SSH_OK && strlen(b64) > 40, "export base64");
  ssh_key back; QT_CHECK(ssh_pki_import_pubkey_base64(b64, SSH_KEYTYPE_ED25519, &back) == SSH_OK, "import");
  QT_CHECK(ssh_key_cmp(pub, back, SSH_KEY_CMP_PUBLIC) == 0, "keys match");
  ssh_session s = ssh_new(); QT_CHECK(s != NULL, "session object"); ssh_free(s);
  free(b64); ssh_key_free(k); ssh_key_free(pub); ssh_key_free(back); ssh_finalize();
  QT_END();
}
