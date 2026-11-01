/* c-blosc: version and the compiled-in codecs (blosclz, lz4, lz4hc, zlib,
 * zstd -- the last three from this tree's lz4/zlib/zstd), then a 4 MB
 * array of int32 counting up, compressed with every codec and every
 * shuffle mode on 4 threads: it must shrink a lot (shuffle makes it nearly
 * constant bytes), carry the right sizes in its header, and decompress
 * bit-exact; blosc_getitem pulls single items out of the middle. Also the
 * context (thread-safe) API and a corrupted buffer being rejected. */
#include "common.h"
#include <blosc.h>
#include <stdint.h>
#include <stdlib.h>

#define N (1024 * 1024)

int main(void) {
  QT_BEGIN("c-blosc");
  blosc_init();
  printf("  info version %s, compressors: %s\n", blosc_get_version_string(), blosc_list_compressors());
  QT_CHECK(!strcmp(blosc_get_version_string(), "1.21.6"), "version 1.21.6");
  const char *codecs[] = { "blosclz", "lz4", "lz4hc", "zlib", "zstd" };
  for (int i = 0; i < 5; i++) {
    char what[96]; snprintf(what, sizeof what, "compressor %s available", codecs[i]);
    QT_CHECK(blosc_compname_to_compcode(codecs[i]) >= 0, what);
  }
  char *lib = NULL, *ver = NULL;
  QT_CHECK(blosc_get_complib_info("zstd", &lib, &ver) >= 0 && lib && !strcmp(lib, "Zstd"), "zstd codec is the Zstd library");
  if (lib) printf("  info zstd: %s %s\n", lib, ver ? ver : "?");
  free(lib); free(ver);

  int32_t *src = malloc(N * sizeof *src), *back = malloc(N * sizeof *back);
  size_t csz = N * sizeof *src + BLOSC_MAX_OVERHEAD;
  char *comp = malloc(csz);
  for (int i = 0; i < N; i++) src[i] = i;
  QT_CHECK(blosc_set_nthreads(4) >= 1 && blosc_get_nthreads() == 4, "4 threads");
  int shuffles[] = { BLOSC_NOSHUFFLE, BLOSC_SHUFFLE, BLOSC_BITSHUFFLE };
  for (int c = 0; c < 5; c++) for (int s = 0; s < 3; s++) {
    char what[128];
    blosc_set_compressor(codecs[c]);
    int cb = blosc_compress(5, shuffles[s], sizeof(int32_t), N * sizeof *src, src, comp, csz);
    size_t nbytes = 0, cbytes = 0, bs = 0;
    if (cb > 0) blosc_cbuffer_sizes(comp, &nbytes, &cbytes, &bs);
    memset(back, 0, N * sizeof *back);
    int db = cb > 0 ? blosc_decompress(comp, back, N * sizeof *back) : -1;
    int same = db == (int)(N * sizeof *src) && !memcmp(src, back, N * sizeof *src);
    printf("  info %-7s shuffle=%d: %d -> %d bytes\n", codecs[c], shuffles[s], (int)(N * sizeof *src), cb);
    snprintf(what, sizeof what, "%s shuffle=%d: round trip bit-exact, header sizes right", codecs[c], shuffles[s]);
    QT_CHECK(cb > 0 && same && nbytes == N * sizeof *src && cbytes == (size_t)cb, what);
    if (shuffles[s] != BLOSC_NOSHUFFLE) {
      snprintf(what, sizeof what, "%s shuffle=%d: compresses a counting int32 array at least 10x", codecs[c], shuffles[s]);
      QT_CHECK(cb > 0 && (size_t)cb * 10 < N * sizeof *src, what);
    }
  }
  blosc_set_compressor("lz4");
  int cb = blosc_compress(9, BLOSC_SHUFFLE, sizeof(int32_t), N * sizeof *src, src, comp, csz);
  int32_t items[3] = { 0, 0, 0 };
  QT_CHECK(cb > 0 && blosc_getitem(comp, 500000, 3, items) == 12 && items[0] == 500000 && items[2] == 500002,
           "blosc_getitem: items 500000..500002 out of the middle");
  memset(back, 0, N * sizeof *back);
  int ccb = blosc_compress_ctx(5, BLOSC_SHUFFLE, sizeof(int32_t), N * sizeof *src, src, comp, csz, "zstd", 0, 2);
  QT_CHECK(ccb > 0 && blosc_decompress_ctx(comp, back, N * sizeof *back, 2) == (int)(N * sizeof *src)
           && !memcmp(src, back, N * sizeof *src), "context API (zstd, 2 threads) round trip");
  /* a header claiming a wrong compressed size must be refused */
  size_t nb = 0;
  QT_CHECK(ccb > 0 && blosc_cbuffer_validate(comp, ccb, &nb) == 0 && nb == N * sizeof *src, "cbuffer_validate accepts the real buffer");
  QT_CHECK(ccb > 0 && blosc_cbuffer_validate(comp, 10, &nb) < 0, "cbuffer_validate rejects a truncated buffer");
  free(src); free(back); free(comp);
  blosc_destroy();
  QT_END();
}
