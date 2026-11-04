/* dnssec-root: the installed root trust anchors checked against the root
 * KSKs' published DS records (RFC 4034: key tag over the DNSKEY RDATA,
 * SHA-256 over owner name + RDATA). Checks: trusted-key.key has exactly the
 * two current KSKs, flags 257 protocol 3 algorithm 8 (RSA/SHA-256), their
 * key tags are 20326 and 38696, their SHA-256 digests are the published DS
 * digests E06D44B8... and 683D2D0A..., and root-anchors.xml next to it
 * lists those same digests. */
#include "common.h"
#include <stdlib.h>
#include <ctype.h>
#include <openssl/evp.h>

#define DIR "/usr/local/share/dnssec-root/"
static const struct { int tag; const char *ds; } KSK[] = {
  { 20326, "E06D44B80B8F1D39A95C0B0D7C65D08458E880409BBC683457104237C7F8EC8D" },
  { 38696, "683D2D0ACB8C9B712A1948B27F741219298D0A450D612C483AF444A4C0FB2B16" },
};
static unsigned keytag(const unsigned char *rd, int n) {   /* RFC 4034 appendix B */
  unsigned long ac = 0;
  for (int i = 0; i < n; i++) ac += (i & 1) ? rd[i] : (unsigned long)rd[i] << 8;
  ac += (ac >> 16) & 0xFFFF;
  return ac & 0xFFFF;
}
int main(void) {
  QT_BEGIN("dnssec-root");
  FILE *f = fopen(DIR "trusted-key.key", "r");
  QT_CHECK(f != NULL, "trusted-key.key is installed");
  if (!f) QT_END();
  char line[2048]; int keys = 0, found[2] = { 0, 0 }, wellformed = 1;
  while (fgets(line, sizeof line, f)) {
    char owner[8], cls[8], type[16], b64[1600]; int flags, proto, alg;
    if (sscanf(line, "%7s %7s %15s %d %d %d %1599s", owner, cls, type, &flags, &proto, &alg, b64) != 7) continue;
    keys++;
    if (strcmp(owner, ".") || strcmp(cls, "IN") || strcmp(type, "DNSKEY") || flags != 257 || proto != 3 || alg != 8) wellformed = 0;
    unsigned char rd[1200]; rd[0] = flags >> 8; rd[1] = flags & 255; rd[2] = proto; rd[3] = alg;
    int kl = EVP_DecodeBlock(rd + 4, (unsigned char *)b64, (int)strlen(b64));
    if (kl <= 0) { wellformed = 0; continue; }
    size_t bl = strlen(b64); kl -= (bl > 0 && b64[bl - 1] == '=') + (bl > 1 && b64[bl - 2] == '=');
    int n = 4 + kl;
    unsigned tag = keytag(rd, n);
    unsigned char msg[1300], md[32]; unsigned ml = 0;
    msg[0] = 0; memcpy(msg + 1, rd, n);            /* owner "." in wire form, then RDATA */
    EVP_Digest(msg, n + 1, md, &ml, EVP_sha256(), NULL);
    char hex[65]; for (int i = 0; i < 32; i++) sprintf(hex + 2 * i, "%02X", md[i]);
    printf("  info key tag %u, %d-byte public key, DS SHA-256 %.16s...\n", tag, kl, hex);
    for (int k = 0; k < 2; k++) if ((int)tag == KSK[k].tag && !strcmp(hex, KSK[k].ds)) found[k] = 1;
  }
  fclose(f);
  QT_CHECK(keys == 2 && wellformed, "two DNSKEY records for \".\": flags 257 (KSK), protocol 3, algorithm 8");
  QT_CHECK(found[0], "KSK-2017: key tag 20326, DS digest E06D44B8...EC8D");
  QT_CHECK(found[1], "KSK-2024: key tag 38696, DS digest 683D2D0A...2B16");
  f = fopen(DIR "root-anchors.xml", "r"); static char x[8192]; size_t xn = f ? fread(x, 1, sizeof x - 1, f) : 0; x[xn] = 0; if (f) fclose(f);
  QT_CHECK(strstr(x, KSK[0].ds) && strstr(x, KSK[1].ds) && strstr(x, "<Zone>.</Zone>"), "root-anchors.xml lists the same DS digests");
  QT_END();
}
