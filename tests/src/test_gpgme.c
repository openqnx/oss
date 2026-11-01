/* GPGME on the installed GnuPG 2.4.9, in a scratch GNUPGHOME:
 *  - gpgme_check_version, the OpenPGP engine (gpg 2.4.9) and gpgconf found;
 *  - an ed25519 key with a cv25519 encryption subkey created without a
 *    passphrase (gpgme_op_createkey / createsubkey), listed with its user id
 *    and algorithms;
 *  - encrypt to it and decrypt: the plaintext comes back, the recipient is
 *    the subkey;
 *  - a detached signature verifies as valid by that fingerprint, and the
 *    same signature over altered data is reported bad (GPG_ERR_BAD_SIGNATURE);
 *  - a clear-text signature; export of the public key and its import into a
 *    second scratch home (one key imported, fingerprint unchanged);
 *  - symmetric encryption with a passphrase given by a callback (loopback).
 * gpg-agent and the other daemons are stopped (gpgconf --kill all) for both
 * homes before the test exits. */
#include "common.h"
#include <stdlib.h>
#include <unistd.h>
#include <locale.h>
#include <gpgme.h>

static char home1[] = "/tmp/qnxport-gpgme1.XXXXXX", home2[] = "/tmp/qnxport-gpgme2.XXXXXX";
static void cleanup(void) {
  char cmd[256];
  for (int i = 0; i < 2; i++) {
    const char *h = i ? home2 : home1;
    if (h[strlen(h) - 1] == 'X') continue;
    snprintf(cmd, sizeof cmd, "GNUPGHOME=%s gpgconf --kill all >/dev/null 2>&1; rm -rf %s", h, h);
    if (system(cmd) != 0) printf("  info cleanup of %s failed\n", h);
  }
}
static gpgme_ctx_t new_ctx(const char *home) {
  gpgme_ctx_t c;
  if (gpgme_new(&c)) return NULL;
  gpgme_set_protocol(c, GPGME_PROTOCOL_OpenPGP);
  gpgme_ctx_set_engine_info(c, GPGME_PROTOCOL_OpenPGP, NULL, home);
  gpgme_set_pinentry_mode(c, GPGME_PINENTRY_MODE_LOOPBACK);
  gpgme_set_armor(c, 1);
  return c;
}
static char *data_str(gpgme_data_t d, size_t *n) {
  return gpgme_data_release_and_get_mem(d, n);
}
static gpgme_error_t passcb(void *hook, const char *uid_hint, const char *info, int prev_bad, int fd) {
  (void)hook; (void)uid_hint; (void)info;
  if (prev_bad) return gpg_error(GPG_ERR_CANCELED);
  const char pw[] = "qnx-symmetric-pass\n";
  return gpgme_io_writen(fd, pw, sizeof pw - 1) ? gpg_error_from_syserror() : 0;
}

int main(void) {
  QT_BEGIN("gpgme");
  setlocale(LC_ALL, "");
  const char *v = gpgme_check_version(NULL);
  printf("  info gpgme %s\n", v ? v : "(null)");
  QT_CHECK(v && !strcmp(v, "2.2.0"), "gpgme_check_version: 2.2.0");
  QT_CHECK(gpgme_engine_check_version(GPGME_PROTOCOL_OpenPGP) == 0, "OpenPGP engine usable");
  if (!mkdtemp(home1) || !mkdtemp(home2)) { QT_CHECK(0, "scratch GNUPGHOMEs"); cleanup(); QT_END(); }
  atexit(cleanup);
  gpgme_engine_info_t ei = NULL;
  gpgme_get_engine_info(&ei);
  for (; ei; ei = ei->next)
    if (ei->protocol == GPGME_PROTOCOL_OpenPGP) {
      printf("  info engine %s %s\n", ei->file_name, ei->version);
      QT_CHECK(ei->version && !strcmp(ei->version, "2.4.9"), "OpenPGP engine: gpg 2.4.9");
    }

  gpgme_ctx_t c = new_ctx(home1);
  QT_CHECK(c != NULL, "context on the first scratch home");
  if (!c) QT_END();
  const char *uid = "QNX GPGME Test <gpgme@qnx.invalid>";
  gpgme_error_t e = gpgme_op_createkey(c, uid, "ed25519", 0, 0, NULL,
                                       GPGME_CREATE_NOPASSWD | GPGME_CREATE_SIGN | GPGME_CREATE_CERT | GPGME_CREATE_NOEXPIRE);
  if (e) printf("  info createkey: %s\n", gpgme_strerror(e));
  gpgme_genkey_result_t gr = e ? NULL : gpgme_op_genkey_result(c);
  char fpr[64] = "";
  if (gr && gr->fpr) snprintf(fpr, sizeof fpr, "%s", gr->fpr);
  printf("  info fingerprint %s\n", fpr);
  QT_CHECK(!e && strlen(fpr) == 40, "ed25519 key created without passphrase (40-hex fingerprint)");

  gpgme_key_t key = NULL;
  e = gpgme_get_key(c, fpr, &key, 0);
  QT_CHECK(!e && key, "gpgme_get_key by fingerprint");
  if (key) {
    e = gpgme_op_createsubkey(c, key, "cv25519", 0, 0, GPGME_CREATE_NOPASSWD | GPGME_CREATE_ENCR | GPGME_CREATE_NOEXPIRE);
    QT_CHECK(!e, "cv25519 encryption subkey added");
    gpgme_key_unref(key); key = NULL;
    gpgme_get_key(c, fpr, &key, 0);
  }
  int nsub = 0, algo_ok = 0, uid_ok = 0;
  if (key) {
    for (gpgme_subkey_t s = key->subkeys; s; s = s->next) nsub++;
    algo_ok = key->subkeys && key->subkeys->pubkey_algo == GPGME_PK_EDDSA && key->subkeys->can_sign &&
              key->subkeys->next && key->subkeys->next->pubkey_algo == GPGME_PK_ECDH && key->subkeys->next->can_encrypt &&
              key->subkeys->next->curve && !strcmp(key->subkeys->next->curve, "cv25519");
    uid_ok = key->uids && key->uids->uid && !strcmp(key->uids->uid, uid);
  }
  QT_CHECK(nsub == 2 && algo_ok, "keylist: EdDSA signing primary, ECDH cv25519 encryption subkey");
  QT_CHECK(uid_ok, "keylist: the user id");

  /* encrypt / decrypt */
  const char msg[] = "QNX says hello through GPGME\n";
  gpgme_data_t in, out;
  gpgme_data_new_from_mem(&in, msg, sizeof msg - 1, 0);
  gpgme_data_new(&out);
  gpgme_key_t rcp[2] = { key, NULL };
  e = key ? gpgme_op_encrypt(c, rcp, GPGME_ENCRYPT_ALWAYS_TRUST, in, out) : gpg_error(GPG_ERR_NO_PUBKEY);
  gpgme_data_release(in);
  size_t clen = 0; char *ct = data_str(out, &clen);
  QT_CHECK(!e && ct && clen > 0 && strstr(ct, "-----BEGIN PGP MESSAGE-----"), "encrypted to the key (ASCII armour)");
  if (!e && ct) {
    gpgme_data_new_from_mem(&in, ct, clen, 1);
    gpgme_data_new(&out);
    e = gpgme_op_decrypt(c, in, out);
    gpgme_decrypt_result_t dr = e ? NULL : gpgme_op_decrypt_result(c);
    size_t plen = 0; char *pt = data_str(out, &plen);
    QT_CHECK(!e && pt && plen == sizeof msg - 1 && !memcmp(pt, msg, plen), "decrypted: the plaintext back");
    QT_CHECK(dr && dr->recipients && key && key->subkeys->next &&
             !strcmp(dr->recipients->keyid, key->subkeys->next->keyid), "decrypt result: recipient is the cv25519 subkey");
    gpgme_free(pt);
    gpgme_data_release(in);
  }
  gpgme_free(ct);

  /* detached signature */
  gpgme_signers_clear(c);
  if (key) gpgme_signers_add(c, key);
  gpgme_data_new_from_mem(&in, msg, sizeof msg - 1, 0);
  gpgme_data_new(&out);
  e = gpgme_op_sign(c, in, out, GPGME_SIG_MODE_DETACH);
  gpgme_data_release(in);
  size_t slen = 0; char *sig = data_str(out, &slen);
  QT_CHECK(!e && sig && strstr(sig, "-----BEGIN PGP SIGNATURE-----"), "detached signature made");
  if (!e && sig) {
    gpgme_data_t s, d;
    gpgme_data_new_from_mem(&s, sig, slen, 0);
    gpgme_data_new_from_mem(&d, msg, sizeof msg - 1, 0);
    e = gpgme_op_verify(c, s, d, NULL);
    gpgme_verify_result_t vr = e ? NULL : gpgme_op_verify_result(c);
    QT_CHECK(vr && vr->signatures && vr->signatures->status == 0 && (vr->signatures->summary & GPGME_SIGSUM_VALID) &&
             !strcmp(vr->signatures->fpr, fpr), "signature verifies: valid, by the key's fingerprint");
    QT_CHECK(vr && vr->signatures && vr->signatures->pubkey_algo == GPGME_PK_EDDSA, "signature algorithm EdDSA");
    gpgme_data_release(s); gpgme_data_release(d);
    gpgme_data_new_from_mem(&s, sig, slen, 0);
    gpgme_data_new_from_mem(&d, "QNX says hello through GPGME!\n", 30, 0);
    e = gpgme_op_verify(c, s, d, NULL);
    vr = e ? NULL : gpgme_op_verify_result(c);
    QT_CHECK(vr && vr->signatures && gpg_err_code(vr->signatures->status) == GPG_ERR_BAD_SIGNATURE,
             "altered data: GPG_ERR_BAD_SIGNATURE");
    gpgme_data_release(s); gpgme_data_release(d);
  }
  gpgme_free(sig);

  /* clear-text signature */
  gpgme_data_new_from_mem(&in, msg, sizeof msg - 1, 0);
  gpgme_data_new(&out);
  e = gpgme_op_sign(c, in, out, GPGME_SIG_MODE_CLEAR);
  gpgme_data_release(in);
  char *cs = data_str(out, &slen);
  QT_CHECK(!e && cs && strstr(cs, "-----BEGIN PGP SIGNED MESSAGE-----") && strstr(cs, "QNX says hello through GPGME"),
           "clear-text signature contains the message");
  gpgme_free(cs);

  /* export, import into a second home */
  gpgme_data_new(&out);
  e = gpgme_op_export(c, fpr, 0, out);
  size_t klen = 0; char *pub = data_str(out, &klen);
  QT_CHECK(!e && pub && strstr(pub, "-----BEGIN PGP PUBLIC KEY BLOCK-----"), "public key exported");
  gpgme_ctx_t c2 = new_ctx(home2);
  if (c2 && pub) {
    gpgme_data_new_from_mem(&in, pub, klen, 0);
    e = gpgme_op_import(c2, in);
    gpgme_import_result_t ir = e ? NULL : gpgme_op_import_result(c2);
    QT_CHECK(ir && ir->imported == 1 && ir->imports && !strcmp(ir->imports->fpr, fpr),
             "imported into the second home: 1 key, same fingerprint");
    gpgme_data_release(in);
  } else QT_CHECK(0, "second context");
  gpgme_free(pub);

  /* symmetric, passphrase from a callback */
  gpgme_set_passphrase_cb(c, passcb, NULL);
  gpgme_data_new_from_mem(&in, msg, sizeof msg - 1, 0);
  gpgme_data_new(&out);
  e = gpgme_op_encrypt(c, NULL, GPGME_ENCRYPT_SYMMETRIC, in, out);
  gpgme_data_release(in);
  ct = data_str(out, &clen);
  if (!e && ct && c2) {
    gpgme_set_passphrase_cb(c2, passcb, NULL);
    gpgme_data_new_from_mem(&in, ct, clen, 0);
    gpgme_data_new(&out);
    e = gpgme_op_decrypt(c2, in, out);
    size_t plen = 0; char *pt = data_str(out, &plen);
    QT_CHECK(!e && pt && plen == sizeof msg - 1 && !memcmp(pt, msg, plen),
             "symmetric encryption (passphrase callback) decrypted in the other home");
    gpgme_free(pt);
    gpgme_data_release(in);
  } else QT_CHECK(0, "symmetric encryption");
  gpgme_free(ct);

  if (key) gpgme_key_unref(key);
  if (c2) gpgme_release(c2);
  gpgme_release(c);
  QT_END();
}
