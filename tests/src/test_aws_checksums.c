/* aws-checksums: CRC32, CRC32C and CRC64/NVME check values, incremental and
 * combined forms, a large buffer through the hardware-accelerated paths, and
 * the xxHash wrappers. */
#include "common.h"
#include <stdlib.h>
#include <aws/common/common.h>
#include <aws/common/byte_buf.h>
#include <aws/checksums/checksums.h>
#include <aws/checksums/crc.h>
#include <aws/checksums/xxhash.h>

/* Bitwise reference CRC-32C, to check the accelerated code on a big buffer. */
static uint32_t ref_crc32c(const uint8_t *p, size_t n) {
  uint32_t c = 0xffffffffu;
  for (size_t i = 0; i < n; ++i) {
    c ^= p[i];
    for (int k = 0; k < 8; ++k) c = (c >> 1) ^ (0x82F63B78u & (0u - (c & 1)));
  }
  return ~c;
}
static int u64_is(const struct aws_byte_buf *b, uint64_t v) {
  if (b->len != 8) return 0;
  uint64_t be = 0, le = 0;
  for (int i = 0; i < 8; ++i) { be = (be << 8) | b->buffer[i]; le |= (uint64_t)b->buffer[i] << (8 * i); }
  if (be != v && le != v) printf("  info got be=%016llx le=%016llx\n", (unsigned long long)be, (unsigned long long)le);
  return be == v || le == v;
}

int main(void) {
  QT_BEGIN("aws-checksums");
  struct aws_allocator *al = aws_default_allocator();
  aws_checksums_library_init(al);
  const uint8_t *chk = (const uint8_t *)"123456789";

  uint32_t c32 = aws_checksums_crc32(chk, 9, 0);
  uint32_t c32c = aws_checksums_crc32c(chk, 9, 0);
  uint64_t c64 = aws_checksums_crc64nvme(chk, 9, 0);
  printf("  info crc32 %08x crc32c %08x crc64nvme %016llx\n", c32, c32c, (unsigned long long)c64);
  QT_CHECK(c32 == 0xCBF43926u, "CRC32 check value");
  QT_CHECK(c32c == 0xE3069283u, "CRC32C check value");
  QT_CHECK(c64 == 0xAE8B14860A799888ULL, "CRC64/NVME check value");
  QT_CHECK(aws_checksums_crc32(chk + 4, 5, aws_checksums_crc32(chk, 4, 0)) == 0xCBF43926u, "CRC32 incremental");
  QT_CHECK(aws_checksums_crc32c_combine(aws_checksums_crc32c(chk, 3, 0), aws_checksums_crc32c(chk + 3, 6, 0), 6) == 0xE3069283u,
           "CRC32C combine");
  QT_CHECK(aws_checksums_crc64nvme_combine(aws_checksums_crc64nvme(chk, 5, 0), aws_checksums_crc64nvme(chk + 5, 4, 0), 4) ==
           0xAE8B14860A799888ULL, "CRC64 combine");

  size_t n = 1 << 20;
  uint8_t *big = malloc(n + 3);
  uint32_t x = 12345;
  for (size_t i = 0; i < n + 3; ++i) { x = x * 1103515245u + 12345u; big[i] = (uint8_t)(x >> 16); }
  /* +3: an unaligned start, so both the head and the SIMD body are used */
  QT_CHECK(aws_checksums_crc32c_ex(big + 3, n, 0) == ref_crc32c(big + 3, n), "CRC32C of 1 MiB matches a bitwise reference");
  free(big);

  struct aws_byte_buf out; aws_byte_buf_init(&out, al, 16);
  QT_CHECK(aws_xxhash64_compute(0, aws_byte_cursor_from_c_str("abc"), &out) == AWS_OP_SUCCESS &&
           u64_is(&out, 0x44BC2CF5AD770999ULL), "XXH64(\"abc\")");
  aws_byte_buf_reset(&out, true);
  QT_CHECK(aws_xxhash3_64_compute(0, aws_byte_cursor_from_c_str(""), &out) == AWS_OP_SUCCESS &&
           u64_is(&out, 0x2D06800538D394C2ULL), "XXH3-64(\"\")");
  aws_byte_buf_reset(&out, true);
  struct aws_xxhash *xh = aws_xxhash64_new(al, 0);
  aws_xxhash_update(xh, aws_byte_cursor_from_c_str("a"));
  aws_xxhash_update(xh, aws_byte_cursor_from_c_str("bc"));
  QT_CHECK(aws_xxhash_finalize(xh, &out) == AWS_OP_SUCCESS && u64_is(&out, 0x44BC2CF5AD770999ULL), "streaming XXH64");
  aws_xxhash_destroy(xh);
  aws_byte_buf_clean_up(&out);
  aws_checksums_library_clean_up();
  QT_END();
}
