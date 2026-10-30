#include "common.h"
#include <string.h>
#include <miniz.h>
/* miniz: CRC-32 check value, deflate round trip, and a ZIP archive written
 * to and read from memory. */
int main(void) {
  QT_BEGIN("miniz");
  QT_CHECK(mz_crc32(MZ_CRC32_INIT, (const unsigned char *)"123456789", 9) == 0xCBF43926u, "crc32(123456789) = cbf43926");
  static unsigned char src[20000], cmp[25000], back[20000];
  for (unsigned i = 0; i < sizeof src; i++) src[i] = (unsigned char)("QNX miniz "[i % 10]);
  mz_ulong cl = sizeof cmp, bl = sizeof back;
  QT_CHECK(mz_compress2(cmp, &cl, src, sizeof src, 9) == MZ_OK, "mz_compress2");
  printf("  info %zu -> %lu bytes\n", sizeof src, (unsigned long)cl);
  QT_CHECK(cl < 500, "repetitive data compresses");
  QT_CHECK(mz_uncompress(back, &bl, cmp, cl) == MZ_OK && bl == sizeof src && memcmp(back, src, bl) == 0, "round trip");
  mz_zip_archive z; memset(&z, 0, sizeof z);
  QT_CHECK(mz_zip_writer_init_heap(&z, 0, 0), "zip writer (heap)");
  mz_zip_writer_add_mem(&z, "hello.txt", "hello, QNX", 10, MZ_BEST_COMPRESSION);
  mz_zip_writer_add_mem(&z, "dir/data.bin", src, sizeof src, MZ_BEST_COMPRESSION);
  void *zbuf; size_t zsz;
  QT_CHECK(mz_zip_writer_finalize_heap_archive(&z, &zbuf, &zsz), "zip finalized");
  mz_zip_writer_end(&z);
  mz_zip_archive r; memset(&r, 0, sizeof r);
  QT_CHECK(mz_zip_reader_init_mem(&r, zbuf, zsz, 0), "zip reader");
  QT_CHECK(mz_zip_reader_get_num_files(&r) == 2, "two entries");
  size_t n; char *h = mz_zip_reader_extract_file_to_heap(&r, "hello.txt", &n, 0);
  QT_CHECK(h && n == 10 && memcmp(h, "hello, QNX", 10) == 0, "hello.txt extracted");
  mz_free(h); mz_zip_reader_end(&r); mz_free(zbuf);
  QT_END();
}
