/* NSS: the softoken's crypto with known answers (SHA-256, SHA-1, HMAC,
 * AES-CBC, ECDSA P-256 sign/verify), then certutil makes a certificate
 * database with a self-signed certificate, which the library opens, finds
 * and verifies. */
#include "common.h"
#include <nss.h>
#include <pk11pub.h>
#include <cert.h>
#include <certdb.h>
#include <keyhi.h>
#include <cryptohi.h>
#include <secerr.h>
#include <secder.h>
#include <prerror.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

static void hex(const unsigned char *p, unsigned n, char *out) {
  for (unsigned i = 0; i < n; i++) sprintf(out + 2 * i, "%02x", p[i]);
}
static int unhex(const char *s, unsigned char *out) {
  int n = 0;
  for (; s[0] && s[1]; s += 2) { unsigned v; sscanf(s, "%2x", &v); out[n++] = (unsigned char)v; }
  return n;
}

static int crypto_checks(void) {
  char h[256];
  unsigned char d[64];

  /* FIPS 180-2 known answers */
  QT_CHECK(PK11_HashBuf(SEC_OID_SHA256, d, (const unsigned char *)"abc", 3) == SECSuccess, "PK11_HashBuf SHA-256");
  hex(d, 32, h);
  QT_CHECK(strcmp(h, "ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad") == 0, "SHA-256(\"abc\") known digest");
  PK11_HashBuf(SEC_OID_SHA1, d, (const unsigned char *)"abc", 3);
  hex(d, 20, h);
  QT_CHECK(strcmp(h, "a9993e364706816aba3e25717850c26c9cd0d89d") == 0, "SHA-1(\"abc\") known digest");

  PK11SlotInfo *slot = PK11_GetInternalSlot();
  QT_CHECK(slot != NULL, "internal (softoken) slot");
  if (!slot) return 1;

  /* RFC 4231 test case 2: HMAC-SHA256, key "Jefe" */
  SECItem k = { siBuffer, (unsigned char *)"Jefe", 4 };
  PK11SymKey *mk = PK11_ImportSymKey(slot, CKM_SHA256_HMAC, PK11_OriginUnwrap, CKA_SIGN, &k, NULL);
  SECItem noparam = { siBuffer, NULL, 0 };
  PK11Context *c = mk ? PK11_CreateContextBySymKey(CKM_SHA256_HMAC, CKA_SIGN, mk, &noparam) : NULL;
  unsigned len = 0;
  const char *msg = "what do ya want for nothing?";
  int ok = c && PK11_DigestBegin(c) == SECSuccess &&
           PK11_DigestOp(c, (const unsigned char *)msg, strlen(msg)) == SECSuccess &&
           PK11_DigestFinal(c, d, &len, sizeof d) == SECSuccess && len == 32;
  if (ok) hex(d, 32, h);
  QT_CHECK(ok && strcmp(h, "5bdcc146bf60754e6a042426089575c75a003f089d2739839dec58b964ec3843") == 0,
           "HMAC-SHA256 (RFC 4231 case 2)");
  if (c) PK11_DestroyContext(c, PR_TRUE);
  if (mk) PK11_FreeSymKey(mk);

  /* NIST SP 800-38A F.2.1/F.2.2: AES-128-CBC, first two blocks */
  unsigned char key[16], iv[16], pt[32], ct[48], back[48];
  unhex("2b7e151628aed2a6abf7158809cf4f3c", key);
  unhex("000102030405060708090a0b0c0d0e0f", iv);
  unhex("6bc1bee22e409f96e93d7e117393172aae2d8a571e03ac9c9eb76fac45af8e51", pt);
  SECItem ki = { siBuffer, key, 16 }, ivi = { siBuffer, iv, 16 };
  PK11SymKey *ak = PK11_ImportSymKey(slot, CKM_AES_CBC, PK11_OriginUnwrap, CKA_ENCRYPT, &ki, NULL);
  unsigned clen = 0, blen = 0;
  ok = ak && PK11_Encrypt(ak, CKM_AES_CBC, &ivi, ct, &clen, sizeof ct, pt, 32) == SECSuccess && clen == 32;
  if (ok) hex(ct, 32, h);
  QT_CHECK(ok && strcmp(h, "7649abac8119b246cee98e9b12e9197d5086cb9b507219ee95db113a917678b2") == 0,
           "AES-128-CBC encrypt (SP 800-38A F.2.1)");
  ok = ak && PK11_Decrypt(ak, CKM_AES_CBC, &ivi, back, &blen, sizeof back, ct, clen) == SECSuccess;
  QT_CHECK(ok && blen == 32 && memcmp(back, pt, 32) == 0, "AES-128-CBC decrypt round trip");
  if (ak) PK11_FreeSymKey(ak);

  /* ECDSA P-256: generate, sign SHA-256("abc"), verify; a changed digest fails */
  SECOidData *curve = SECOID_FindOIDByTag(SEC_OID_ANSIX962_EC_PRIME256V1);
  unsigned char pbuf[2 + 16];
  SECItem ecparams = { siBuffer, pbuf, 0 };
  pbuf[0] = 0x06; pbuf[1] = (unsigned char)curve->oid.len;
  memcpy(pbuf + 2, curve->oid.data, curve->oid.len);
  ecparams.len = 2 + curve->oid.len;
  SECKEYPublicKey *pub = NULL;
  SECKEYPrivateKey *priv = PK11_GenerateKeyPair(slot, CKM_EC_KEY_PAIR_GEN, &ecparams, &pub, PR_FALSE, PR_FALSE, NULL);
  QT_CHECK(priv && pub, "EC P-256 key pair generated");
  if (priv && pub) {
    PK11_HashBuf(SEC_OID_SHA256, d, (const unsigned char *)"abc", 3);
    unsigned char sigbuf[128];
    SECItem dig = { siBuffer, d, 32 }, sig = { siBuffer, sigbuf, PK11_SignatureLen(priv) };
    QT_CHECK(sig.len == 64 && PK11_Sign(priv, &sig, &dig) == SECSuccess, "ECDSA sign (64-byte signature)");
    QT_CHECK(PK11_Verify(pub, &sig, &dig, NULL) == SECSuccess, "ECDSA verify");
    d[0] ^= 1;
    QT_CHECK(PK11_Verify(pub, &sig, &dig, NULL) != SECSuccess, "ECDSA verify rejects another digest");
  }
  if (priv) SECKEY_DestroyPrivateKey(priv);
  if (pub) SECKEY_DestroyPublicKey(pub);

  unsigned char rnd[32] = {0}, zero[32] = {0};
  QT_CHECK(PK11_GenerateRandom(rnd, sizeof rnd) == SECSuccess && memcmp(rnd, zero, 32) != 0, "PK11_GenerateRandom");
  PK11_FreeSlot(slot);
  return 0;
}

int main(void) {
  QT_BEGIN("nss");
  printf("  info NSS %s\n", NSS_GetVersion());
  QT_CHECK(NSS_VersionCheck("3.131"), "NSS_VersionCheck(3.131)");

  /* 1. no database: the softoken is loaded (dlopen'ed) on its own */
  SECStatus rv = NSS_NoDB_Init(NULL);
  QT_CHECK(rv == SECSuccess, "NSS_NoDB_Init");
  if (rv != SECSuccess) { printf("  info error %d\n", PR_GetError()); QT_END(); }
  crypto_checks();
  QT_CHECK(NSS_Shutdown() == SECSuccess, "NSS_Shutdown");

  /* 2. certutil: a new database with a self-signed EC certificate */
  char dir[256], cmd[1024], spec[300];
  const char *tmp = getenv("TMPDIR");
  snprintf(dir, sizeof dir, "%s/qnxport-nss.XXXXXX", tmp && *tmp ? tmp : "/tmp");
  if (!mkdtemp(dir)) { QT_CHECK(0, "mkdtemp"); QT_END(); }
  /* certutil's key generation seed (-z); any bytes will do */
  snprintf(cmd, sizeof cmd, "%s/noise", dir);
  FILE *nf = fopen(cmd, "w");
  for (int i = 0; nf && i < 64; i++) fputc((i * 73 + 41) & 0xff, nf);
  if (nf) fclose(nf);
  snprintf(cmd, sizeof cmd, "certutil -N -d sql:%s --empty-password > %s/out.txt 2>&1", dir, dir);
  QT_CHECK(system(cmd) == 0, "certutil -N (new database, empty password)");
  snprintf(cmd, sizeof cmd,
           "certutil -S -d sql:%s -n qnxtest -s 'CN=QNX NSS Test,O=openqnx' -x -t 'CT,C,C' -k ec -q nistp256 "
           "-Z SHA256 -v 12 -m 4242 -z %s/noise >> %s/out.txt 2>&1", dir, dir, dir);
  int r = system(cmd);
  QT_CHECK(r == 0, "certutil -S (self-signed P-256 certificate)");
  snprintf(cmd, sizeof cmd, "certutil -L -d sql:%s > %s/list.txt 2>&1", dir, dir);
  int lr = system(cmd);
  char list[4096] = "";
  snprintf(cmd, sizeof cmd, "%s/list.txt", dir);
  FILE *lf = fopen(cmd, "r");
  if (lf) { list[fread(list, 1, sizeof list - 1, lf)] = 0; fclose(lf); }
  char *line = strstr(list, "qnxtest ");
  QT_CHECK(lr == 0 && line && strncmp(line + strspn(line + 7, " ") + 7, "CTu,Cu,Cu", 9) == 0,
           "certutil -L lists qnxtest, trust CTu,Cu,Cu (u: own key)");
  if (r != 0) { snprintf(cmd, sizeof cmd, "cat %s/out.txt", dir); system(cmd); }

  /* 3. the library opens the database and checks the certificate */
  snprintf(spec, sizeof spec, "sql:%s", dir);
  rv = NSS_Init(spec);
  QT_CHECK(rv == SECSuccess, "NSS_Init(sql:<dir>)");
  if (rv == SECSuccess) {
    CERTCertificate *cert = CERT_FindCertByNickname(CERT_GetDefaultCertDB(), "qnxtest");
    QT_CHECK(cert != NULL, "CERT_FindCertByNickname(qnxtest)");
    if (cert) {
      QT_CHECK(strcmp(cert->subjectName, "CN=QNX NSS Test,O=openqnx") == 0, "subject CN=QNX NSS Test,O=openqnx");
      QT_CHECK(strcmp(cert->subjectName, cert->issuerName) == 0, "issuer == subject (self-signed)");
      QT_CHECK(DER_GetInteger(&cert->serialNumber) == 4242, "serial number 4242");
      QT_CHECK(CERT_VerifySignedData(&cert->signatureWrap, cert, PR_Now(), NULL) == SECSuccess,
               "signature verifies with its own key");
      QT_CHECK(CERT_VerifyCertNow(CERT_GetDefaultCertDB(), cert, PR_TRUE, certUsageSSLCA, NULL) == SECSuccess,
               "CERT_VerifyCertNow: trusted CA (usage SSL CA)");
      SECKEYPrivateKey *pk = PK11_FindKeyByAnyCert(cert, NULL);
      QT_CHECK(pk != NULL && pk->keyType == ecKey, "its EC private key is in the database");
      if (pk) SECKEY_DestroyPrivateKey(pk);
      CERT_DestroyCertificate(cert);
    }
    QT_CHECK(NSS_Shutdown() == SECSuccess, "NSS_Shutdown");
  }
  snprintf(cmd, sizeof cmd, "rm -rf %s", dir);
  system(cmd);
  QT_END();
}
