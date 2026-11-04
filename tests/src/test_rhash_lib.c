/* rhash, LibRHash: known digests (the standard test vectors, as Python's
 * hashlib and zlib compute them) of "abc" and of a 1 MiB buffer fed in
 * pieces; one rhash_msg() per algorithm, a multi-hash context updated in
 * chunks, hex printing, the algorithm names. */
#include "common.h"
#include <stdlib.h>
#include <rhash.h>

static const char *hex(unsigned id, const unsigned char *d) {
  static char out[260];
  rhash_print_bytes(out, d, rhash_get_digest_size(id), RHPR_HEX);
  return out;
}

int main(void) {
  unsigned char d[130];
  char out[260];
  static const struct { unsigned id; const char *abc; } v[] = {
    { RHASH_CRC32,    "352441c2" },
    { RHASH_MD5,      "900150983cd24fb0d6963f7d28e17f72" },
    { RHASH_SHA1,     "a9993e364706816aba3e25717850c26c9cd0d89d" },
    { RHASH_SHA256,   "ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad" },
    { RHASH_SHA3_256, "3a985da74fe225b2045c172d6bd390bd855f086e3e9d525b46bfe24511431532" },
  };
  unsigned i, ids[2] = { RHASH_SHA256, RHASH_MD5 };
  unsigned char *buf; rhash ctx;
  QT_BEGIN("rhash_lib");
  rhash_library_init();
  for (i = 0; i < sizeof v / sizeof v[0]; i++) {
    char what[120];
    int ok = rhash_msg(v[i].id, "abc", 3, d) == 0 && strcmp(hex(v[i].id, d), v[i].abc) == 0;
    snprintf(what, sizeof what, "%s(\"abc\") = %.16s...", rhash_get_name(v[i].id), v[i].abc);
    QT_CHECK(ok, what);
  }
  QT_CHECK(rhash_msg(RHASH_SHA512, "abc", 3, d) == 0 && strncmp(hex(RHASH_SHA512, d), "ddaf35a193617abacc417349ae204131", 32) == 0, "SHA-512(\"abc\")");
  QT_CHECK(rhash_msg(RHASH_BLAKE2B, "abc", 3, d) == 0 && strncmp(hex(RHASH_BLAKE2B, d), "ba80a53f981c4d0d6a2797b69f12f6e9", 32) == 0, "BLAKE2b-512(\"abc\")");
  /* 1 MiB of bytes i*7 mod 251, fed in uneven chunks to SHA-256 and MD5 at once */
  buf = malloc(1 << 20);
  for (i = 0; i < (1u << 20); i++) buf[i] = (unsigned char)((i * 7u) % 251u);
  ctx = rhash_init_multi(2, ids);
  QT_CHECK(ctx != NULL, "rhash_init_multi(SHA-256, MD5)");
  for (i = 0; i < (1u << 20); ) { unsigned n = (1u << 20) - i < 4099 ? (1u << 20) - i : 4099; rhash_update(ctx, buf + i, n); i += n; }
  rhash_final(ctx, NULL);
  rhash_print(out, ctx, RHASH_SHA256, RHPR_HEX);
  QT_CHECK(strcmp(out, "e76e4c02227083fd12207b7bc85287bb9e02a618fed3bd8eab1bc2daeda2fb53") == 0, "SHA-256 of the 1 MiB buffer in 4099-byte chunks");
  rhash_print(out, ctx, RHASH_MD5, RHPR_HEX);
  QT_CHECK(strcmp(out, "26e2437d8c01f4bceded84f9309ac88f") == 0, "MD5 of the same buffer from the same context");
  rhash_free(ctx); free(buf);
  QT_CHECK(strcmp(rhash_get_name(RHASH_SHA3_256), "SHA3-256") == 0 && rhash_get_digest_size(RHASH_SHA3_256) == 32, "name and digest size of SHA3-256");
  QT_END();
}
