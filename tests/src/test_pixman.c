/* pixman: composite one image onto another through the generic path. */
#include "common.h"
#include <stdlib.h>
#include <pixman.h>
#define W 32
#define H 16
int main(void) {
  QT_BEGIN("pixman");
  printf("  info pixman %s\n", pixman_version_string());
  uint32_t *dst = calloc(W * H, 4);
  uint32_t *src = malloc(W * H * 4);
  for (int i = 0; i < W * H; ++i) src[i] = 0x80FF0000u;  /* 50% alpha red, premultiplied-ish */
  pixman_image_t *di = pixman_image_create_bits(PIXMAN_a8r8g8b8, W, H, dst, W * 4);
  pixman_image_t *si = pixman_image_create_bits(PIXMAN_a8r8g8b8, W, H, src, W * 4);
  QT_CHECK(di && si, "pixman_image_create_bits");
  QT_CHECK(pixman_image_get_width(di) == W && pixman_image_get_height(di) == H,
           "image geometry");
  pixman_image_composite32(PIXMAN_OP_OVER, si, NULL, di, 0, 0, 0, 0, 0, 0, W, H);
  QT_CHECK(dst[0] != 0, "composite wrote pixels");
  QT_CHECK((dst[0] >> 24) == 0x80, "alpha channel came through OVER");
  /* A clipped composite must leave the rest of the destination untouched. */
  uint32_t *dst2 = calloc(W * H, 4);
  pixman_image_t *di2 = pixman_image_create_bits(PIXMAN_a8r8g8b8, W, H, dst2, W * 4);
  pixman_image_composite32(PIXMAN_OP_SRC, si, NULL, di2, 0, 0, 0, 0, 4, 4, 8, 8);
  QT_CHECK(dst2[0] == 0, "pixel outside the clip is untouched");
  QT_CHECK(dst2[4 * W + 4] == 0x80FF0000u, "pixel inside the clip was written");
  pixman_image_unref(di); pixman_image_unref(si); pixman_image_unref(di2);
  free(dst); free(src); free(dst2);
  QT_END();
}
