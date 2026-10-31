/* ca-certificates: the Mozilla CA bundle as OpenSSL's trust store. OpenSSL's
 * default verify paths must find it (so every TLS client on the target
 * trusts these roots without configuration); it holds 100+ roots, among
 * them ISRG Root X1, which verifies as a trust anchor. A chain from a CA
 * made here is rejected against the bundle alone and accepted once that CA
 * is added; the leaf matches its host name. */
#include "common.h"
#include <openssl/evp.h>
#include <openssl/pem.h>
#include <openssl/x509.h>
#include <openssl/x509v3.h>
#include <openssl/err.h>
#include <stdlib.h>
#include <unistd.h>

static int count_store(X509_STORE *st) {
  STACK_OF(X509_OBJECT) *objs = X509_STORE_get0_objects(st);
  int n = 0;
  for (int i = 0; i < sk_X509_OBJECT_num(objs); i++) n += X509_OBJECT_get_type(sk_X509_OBJECT_value(objs, i)) == X509_LU_X509;
  return n;
}
static int verify(X509_STORE *st, X509 *leaf, int no_time, int *err, int *depth) {
  X509_STORE_CTX *c = X509_STORE_CTX_new();
  X509_STORE_CTX_init(c, st, leaf, NULL);
  if (no_time) X509_STORE_CTX_set_flags(c, X509_V_FLAG_NO_CHECK_TIME);
  int ok = X509_verify_cert(c) == 1;
  *err = X509_STORE_CTX_get_error(c);
  *depth = sk_X509_num(X509_STORE_CTX_get0_chain(c));
  X509_STORE_CTX_free(c);
  return ok;
}
static X509 *make_cert(const char *cn, EVP_PKEY *key, X509 *issuer, EVP_PKEY *ikey, int ca, long serial) {
  X509 *x = X509_new();
  X509_set_version(x, 2);
  ASN1_INTEGER_set(X509_get_serialNumber(x), serial);
  X509_gmtime_adj(X509_getm_notBefore(x), -86400);
  X509_gmtime_adj(X509_getm_notAfter(x), 86400L * 365);
  X509_set_pubkey(x, key);
  X509_NAME *nm = X509_get_subject_name(x);
  X509_NAME_add_entry_by_txt(nm, "O", MBSTRING_ASC, (const unsigned char *)"QNX port tests", -1, -1, 0);
  X509_NAME_add_entry_by_txt(nm, "CN", MBSTRING_ASC, (const unsigned char *)cn, -1, -1, 0);
  X509_set_issuer_name(x, issuer ? X509_get_subject_name(issuer) : nm);
  X509V3_CTX v3; X509V3_set_ctx_nodb(&v3); X509V3_set_ctx(&v3, issuer ? issuer : x, x, NULL, NULL, 0);
  const char *exts[][2] = { { "basicConstraints", ca ? "critical,CA:TRUE" : "CA:FALSE" },
                            { "keyUsage", ca ? "critical,keyCertSign,cRLSign" : "critical,digitalSignature" },
                            { "subjectKeyIdentifier", "hash" },
                            { "subjectAltName", ca ? NULL : "DNS:qnx.test,DNS:*.qnx.test" } };
  for (int i = 0; i < 4; i++) {
    if (!exts[i][1]) continue;
    X509_EXTENSION *e = X509V3_EXT_conf(NULL, &v3, exts[i][0], exts[i][1]);
    if (e) { X509_add_ext(x, e, -1); X509_EXTENSION_free(e); }
  }
  X509_sign(x, ikey ? ikey : key, EVP_sha256());
  return x;
}

int main(void) {
  QT_BEGIN("ca-certificates");
  const char *bundle = "/usr/local/ssl/cert.pem";
  if (getenv("QNX_PORT_CA_BUNDLE")) bundle = getenv("QNX_PORT_CA_BUNDLE");
  printf("  info OpenSSL %s; default cert file %s, dir %s\n", OpenSSL_version(OPENSSL_VERSION), X509_get_default_cert_file(), X509_get_default_cert_dir());
  QT_CHECK(access(bundle, R_OK) == 0, "bundle /usr/local/ssl/cert.pem installed");

  X509_STORE *st = X509_STORE_new();
  QT_CHECK(X509_STORE_load_file(st, bundle) == 1, "X509_STORE_load_file(bundle)");
  int n = count_store(st);
  printf("  info %d certificates in the bundle\n", n);
  QT_CHECK(n >= 100, "100 or more root certificates");
  const char *alt[] = { "/usr/local/etc/ssl/certs/ca-certificates.crt", "/usr/local/ssl/certs/ca-certificates.crt" };
  for (int i = 0; i < 2; i++) {
    X509_STORE *s2 = X509_STORE_new();
    char what[160]; snprintf(what, sizeof what, "%s: the same %d certificates", alt[i], n);
    QT_CHECK(X509_STORE_load_file(s2, alt[i]) == 1 && count_store(s2) == n, what);
    X509_STORE_free(s2);
  }
  /* what any OpenSSL client gets with SSL_CTX_set_default_verify_paths() */
  X509_STORE *def = X509_STORE_new();
  X509_STORE_set_default_paths(def);
  ERR_clear_error();
  int nd = count_store(def);
  printf("  info default verify paths loaded %d certificate(s)\n", nd);
  QT_CHECK(nd == n, "OpenSSL's default verify paths load this bundle");
  X509_STORE_free(def);

  /* ISRG Root X1, found by name in the PEM file */
  FILE *f = fopen(bundle, "r"); X509 *x, *isrg = NULL; int pem = 0;
  while (f && (x = PEM_read_X509(f, NULL, NULL, NULL))) {
    pem++;
    char cn[256] = "";
    X509_NAME_get_text_by_NID(X509_get_subject_name(x), NID_commonName, cn, sizeof cn);
    if (!isrg && !strcmp(cn, "ISRG Root X1")) isrg = x; else X509_free(x);
  }
  if (f) fclose(f);
  ERR_clear_error();
  QT_CHECK(pem == n, "every PEM block parses as a certificate");
  QT_CHECK(isrg != NULL, "ISRG Root X1 (Let's Encrypt) present");
  int err = 0, depth = 0;
  if (isrg) {
    EVP_PKEY *pk = X509_get0_pubkey(isrg);
    QT_CHECK(X509_verify(isrg, pk) == 1 && EVP_PKEY_get_bits(pk) == 4096, "ISRG Root X1: self-signed, RSA 4096, signature verifies");
    QT_CHECK(verify(st, isrg, 1, &err, &depth) && depth == 1, "ISRG Root X1 verifies as a trust anchor of the bundle");
    X509_free(isrg);
  }

  /* a CA of our own */
  EVP_PKEY *cakey = EVP_EC_gen("P-256"), *leafkey = EVP_EC_gen("P-256");
  X509 *ca = make_cert("QNX Test Root CA", cakey, NULL, NULL, 1, 1);
  X509 *leaf = make_cert("qnx.test", leafkey, ca, cakey, 0, 2);
  QT_CHECK(ca && leaf && X509_verify(leaf, cakey) == 1, "made a CA (EC P-256) and a leaf it signed");
  int ok = verify(st, leaf, 1, &err, &depth);
  printf("  info against the bundle alone: %s\n", X509_verify_cert_error_string(err));
  QT_CHECK(!ok && err == X509_V_ERR_UNABLE_TO_GET_ISSUER_CERT_LOCALLY, "leaf rejected by the bundle alone (issuer unknown)");
  X509_STORE_add_cert(st, ca);
  ok = verify(st, leaf, 1, &err, &depth);
  printf("  info with the CA added: %s, chain depth %d\n", X509_verify_cert_error_string(err), depth);
  QT_CHECK(ok && depth == 2, "accepted once the CA is trusted: chain leaf -> CA");
  QT_CHECK(X509_check_host(leaf, "qnx.test", 0, 0, NULL) == 1 && X509_check_host(leaf, "www.qnx.test", 0, 0, NULL) == 1
           && X509_check_host(leaf, "example.org", 0, 0, NULL) != 1, "host name check: qnx.test, *.qnx.test, not example.org");
  X509_free(leaf); X509_free(ca); EVP_PKEY_free(cakey); EVP_PKEY_free(leafkey);
  X509_STORE_free(st);
  QT_END();
}
