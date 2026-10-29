/* libavif: encode an RGBA image to AVIF and decode it back, which drives the
 * AV1 encoder (libaom) and decoder (dav1d) through libavif's own plumbing. */
#include "common.h"
#include <stdlib.h>
#include <avif/avif.h>
#define W 64
#define H 48
int main(void) {
  QT_BEGIN("libavif");
  printf("  info libavif %s\n", avifVersion());
  char codecs[256];
  avifCodecVersions(codecs);
  printf("  info codecs: %s\n", codecs);
  QT_CHECK(avifCodecName(AVIF_CODEC_CHOICE_AUTO, AVIF_CODEC_FLAG_CAN_ENCODE) != NULL,
           "an AV1 encoder is available");
  QT_CHECK(avifCodecName(AVIF_CODEC_CHOICE_AUTO, AVIF_CODEC_FLAG_CAN_DECODE) != NULL,
           "an AV1 decoder is available");

  avifImage *img = avifImageCreate(W, H, 8, AVIF_PIXEL_FORMAT_YUV444);
  QT_CHECK(img != NULL, "avifImageCreate");
  if (!img) QT_END();
  uint8_t rgba[H][W][4];
  for (int y = 0; y < H; ++y) for (int x = 0; x < W; ++x) {
    rgba[y][x][0] = (uint8_t)(x * 4); rgba[y][x][1] = (uint8_t)(y * 5);
    rgba[y][x][2] = 0x50; rgba[y][x][3] = 0xFF;
  }
  avifRGBImage rgb;
  avifRGBImageSetDefaults(&rgb, img);
  rgb.format = AVIF_RGB_FORMAT_RGBA;
  rgb.depth = 8;
  rgb.pixels = (uint8_t *)rgba;
  rgb.rowBytes = W * 4;
  QT_CHECK(avifImageRGBToYUV(img, &rgb) == AVIF_RESULT_OK, "avifImageRGBToYUV");

  avifEncoder *enc = avifEncoderCreate();
  QT_CHECK(enc != NULL, "avifEncoderCreate");
  enc->quality = AVIF_QUALITY_LOSSLESS;
  enc->qualityAlpha = AVIF_QUALITY_LOSSLESS;
  enc->speed = AVIF_SPEED_FASTEST;
  enc->maxThreads = 1;
  avifRWData out = AVIF_DATA_EMPTY;
  avifResult res = avifEncoderWrite(enc, img, &out);
  QT_CHECK(res == AVIF_RESULT_OK, "avifEncoderWrite");
  if (res != AVIF_RESULT_OK) printf("  info %s\n", avifResultToString(res));
  QT_CHECK(out.size > 0, "AVIF bytes produced");
  avifEncoderDestroy(enc);

  avifDecoder *dec = avifDecoderCreate();
  dec->maxThreads = 1;
  QT_CHECK(avifDecoderSetIOMemory(dec, out.data, out.size) == AVIF_RESULT_OK,
           "avifDecoderSetIOMemory");
  QT_CHECK(avifDecoderParse(dec) == AVIF_RESULT_OK, "avifDecoderParse");
  QT_CHECK(dec->image && dec->image->width == W && dec->image->height == H,
           "decoded dimensions match");
  QT_CHECK(avifDecoderNextImage(dec) == AVIF_RESULT_OK, "avifDecoderNextImage");
  uint8_t back[H][W][4];
  avifRGBImage rgb2;
  avifRGBImageSetDefaults(&rgb2, dec->image);
  rgb2.format = AVIF_RGB_FORMAT_RGBA;
  rgb2.depth = 8;
  rgb2.pixels = (uint8_t *)back;
  rgb2.rowBytes = W * 4;
  QT_CHECK(avifImageYUVToRGB(dec->image, &rgb2) == AVIF_RESULT_OK, "avifImageYUVToRGB");
  long diff = 0;
  for (int i = 0; i < W * H * 4; ++i)
    diff += labs((long)((uint8_t *)rgba)[i] - (long)((uint8_t *)back)[i]);
  QT_CHECK(diff / (W * H * 4) < 4, "decoded pixels are close to the original");
  avifDecoderDestroy(dec);
  avifRWDataFree(&out);
  avifImageDestroy(img);
  QT_END();
}
