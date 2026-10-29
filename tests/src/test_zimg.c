/* zimg: colourspace conversion and scaling used by video pipelines. */
#include "common.h"
#include <stdlib.h>
#include <malloc.h>
#include <zimg.h>

/* zimg documents the contract: "The image address and stride must be a multiple
 * of the alignment imposed by the host CPU architecture. On x86 and AMD64, this
 * is 32 bytes." plain malloc() gives 16, so its AVX kernels then execute
 * aligned loads against a misaligned address and the process dies -- which is
 * exactly what happened here on x86_64 while aarch64 (16-byte NEON) passed. */
#define ZALIGN 64
static void *aligned_alloc_z(size_t n) {
  void *p = NULL;
  n = (n + ZALIGN - 1) & ~(size_t)(ZALIGN - 1);
  if (posix_memalign(&p, ZALIGN, n) != 0) return NULL;
  memset(p, 0, n);
  return p;
}
/* Round a stride up to the alignment as well. */
#define ZSTRIDE(w) (((size_t)(w) + ZALIGN - 1) & ~(size_t)(ZALIGN - 1))
#define W 128
#define H 96
int main(void) {
  QT_BEGIN("zimg");
  unsigned ver[3];
  zimg_get_version_info(&ver[0], &ver[1], &ver[2]);
  printf("  info zimg %u.%u.%u (API %u)\n", ver[0], ver[1], ver[2], zimg_get_api_version(NULL, NULL));
  zimg_image_format src, dst;
  zimg_image_format_default(&src, ZIMG_API_VERSION);
  zimg_image_format_default(&dst, ZIMG_API_VERSION);
  src.width = W; src.height = H;
  src.pixel_type = ZIMG_PIXEL_BYTE;
  src.color_family = ZIMG_COLOR_YUV;
  src.subsample_w = 1; src.subsample_h = 1;
  src.depth = 8;
  dst = src;
  dst.width = W / 2; dst.height = H / 2;
  zimg_graph_builder_params params;
  zimg_graph_builder_params_default(&params, ZIMG_API_VERSION);
  params.resample_filter = ZIMG_RESIZE_BICUBIC;
  zimg_filter_graph *graph = zimg_filter_graph_build(&src, &dst, &params);
  QT_CHECK(graph != NULL, "zimg_filter_graph_build");
  if (!graph) {
    char err[256]; zimg_get_last_error(err, sizeof err);
    printf("  info %s\n", err);
    QT_END();
  }
  size_t tmp_size = 0;
  QT_CHECK(zimg_filter_graph_get_tmp_size(graph, &tmp_size) == 0,
           "zimg_filter_graph_get_tmp_size");
  const size_t s_luma = ZSTRIDE(W), s_chroma = ZSTRIDE(W / 2);
  const size_t d_luma = ZSTRIDE(W / 2), d_chroma = ZSTRIDE(W / 4);
  void *tmp = aligned_alloc_z(tmp_size ? tmp_size : ZALIGN);
  unsigned char *sy = aligned_alloc_z(s_luma * H);
  unsigned char *su = aligned_alloc_z(s_chroma * H / 2);
  unsigned char *sv = aligned_alloc_z(s_chroma * H / 2);
  unsigned char *dy = aligned_alloc_z(d_luma * H / 2);
  unsigned char *du = aligned_alloc_z(d_chroma * H / 4);
  unsigned char *dv = aligned_alloc_z(d_chroma * H / 4);
  QT_CHECK(tmp && sy && su && sv && dy && du && dv, "aligned buffers allocated");
  if (!(tmp && sy && su && sv && dy && du && dv)) QT_END();
  memset(su, 100, s_chroma * H / 2); memset(sv, 150, s_chroma * H / 2);
  for (int y = 0; y < H; ++y)
    for (int x = 0; x < W; ++x) sy[y * s_luma + x] = (unsigned char)(x * 2);
  zimg_image_buffer_const sbuf = { ZIMG_API_VERSION };
  zimg_image_buffer dbuf = { ZIMG_API_VERSION };
  sbuf.plane[0].data = sy; sbuf.plane[0].stride = (ptrdiff_t)s_luma;   sbuf.plane[0].mask = ZIMG_BUFFER_MAX;
  sbuf.plane[1].data = su; sbuf.plane[1].stride = (ptrdiff_t)s_chroma; sbuf.plane[1].mask = ZIMG_BUFFER_MAX;
  sbuf.plane[2].data = sv; sbuf.plane[2].stride = (ptrdiff_t)s_chroma; sbuf.plane[2].mask = ZIMG_BUFFER_MAX;
  dbuf.plane[0].data = dy; dbuf.plane[0].stride = (ptrdiff_t)d_luma;   dbuf.plane[0].mask = ZIMG_BUFFER_MAX;
  dbuf.plane[1].data = du; dbuf.plane[1].stride = (ptrdiff_t)d_chroma; dbuf.plane[1].mask = ZIMG_BUFFER_MAX;
  dbuf.plane[2].data = dv; dbuf.plane[2].stride = (ptrdiff_t)d_chroma; dbuf.plane[2].mask = ZIMG_BUFFER_MAX;
  QT_CHECK(zimg_filter_graph_process(graph, &sbuf, &dbuf, tmp, NULL, NULL, NULL, NULL) == 0,
           "zimg_filter_graph_process");
  long sum = 0;
  for (int y = 0; y < H / 2; ++y)
    for (int x = 0; x < W / 2; ++x) sum += dy[y * d_luma + x];
  QT_CHECK(sum > 0, "the destination plane was written");
  QT_CHECK(du[0] == 100 && dv[0] == 150, "flat chroma survives the resize");
  zimg_filter_graph_free(graph);
  free(tmp); free(sy); free(su); free(sv); free(dy); free(du); free(dv);
  QT_END();
}
