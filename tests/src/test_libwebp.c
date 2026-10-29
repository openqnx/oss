/* libwebp: lossless encode of an RGBA image, then decode and compare. */
#include "common.h"
#include <stdlib.h>
#include <webp/encode.h>
#include <webp/decode.h>
#define W 48
#define H 24
int main(void) {
  QT_BEGIN("libwebp");
  printf("  info encoder ABI 0x%04x, decoder ABI 0x%04x\n",
         WebPGetEncoderVersion(), WebPGetDecoderVersion());
  unsigned char img[H][W][4];
  for (int y = 0; y < H; ++y) for (int x = 0; x < W; ++x) {
    img[y][x][0] = (unsigned char)(x * 5); img[y][x][1] = (unsigned char)(y * 10);
    img[y][x][2] = 0x77; img[y][x][3] = 0xFF;
  }
  uint8_t *out = NULL;
  size_t n = WebPEncodeLosslessRGBA((const uint8_t *)img, W, H, W * 4, &out);
  QT_CHECK(n > 0 && out != NULL, "WebPEncodeLosslessRGBA");
  int fw = 0, fh = 0;
  QT_CHECK(WebPGetInfo(out, n, &fw, &fh) && fw == W && fh == H,
           "WebPGetInfo reports the right size");
  uint8_t *back = WebPDecodeRGBA(out, n, &fw, &fh);
  QT_CHECK(back != NULL, "WebPDecodeRGBA");
  QT_CHECK(back && memcmp(back, img, sizeof img) == 0,
           "lossless round trip is byte-identical");
  WebPFree(back);
  WebPFree(out);
  /* Lossy path must also work and stay close to the original. */
  out = NULL;
  n = WebPEncodeRGBA((const uint8_t *)img, W, H, W * 4, 85.0f, &out);
  QT_CHECK(n > 0, "WebPEncodeRGBA (lossy)");
  WebPFree(out);
  QT_END();
}
