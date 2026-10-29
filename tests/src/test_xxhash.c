/* xxhash: one-shot and streaming digests must agree, and be endian-stable. */
#include "common.h"
#include <xxhash.h>
int main(void) {
  QT_BEGIN("xxhash");
  const char msg[] = "The quick brown fox jumps over the lazy dog";
  size_t n = sizeof msg - 1;
  XXH64_hash_t one = XXH64(msg, n, 0);
  XXH64_state_t *st = XXH64_createState();
  QT_CHECK(st != NULL, "XXH64_createState");
  XXH64_reset(st, 0);
  XXH64_update(st, msg, 10);
  XXH64_update(st, msg + 10, n - 10);
  QT_CHECK(XXH64_digest(st) == one, "streaming digest == one-shot digest");
  XXH64_freeState(st);
  /* Reference vector from the xxHash specification. */
  QT_CHECK(XXH64("", 0, 0) == 0xEF46DB3751D8E999ULL, "XXH64(\"\") reference vector");
  QT_CHECK(XXH32("", 0, 0) == 0x02CC5D05U, "XXH32(\"\") reference vector");
  XXH128_hash_t h = XXH3_128bits(msg, n);
  QT_CHECK(h.low64 != 0 || h.high64 != 0, "XXH3 128-bit digest");
  QT_END();
}
