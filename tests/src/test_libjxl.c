/* libjxl: encode an RGBA image to JPEG XL and decode it back losslessly. */
#include "common.h"
#include <stdlib.h>
#include <jxl/encode.h>
#include <jxl/decode.h>
#define W 48
#define H 32
int main(void) {
  QT_BEGIN("libjxl");
  uint32_t ver = JxlEncoderVersion();
  printf("  info libjxl %u.%u.%u\n", ver / 1000000, (ver / 1000) % 1000, ver % 1000);
  uint8_t img[H][W][4];
  for (int y = 0; y < H; ++y) for (int x = 0; x < W; ++x) {
    img[y][x][0] = (uint8_t)(x * 5); img[y][x][1] = (uint8_t)(y * 7);
    img[y][x][2] = 0x60; img[y][x][3] = 0xFF;
  }
  JxlEncoder *enc = JxlEncoderCreate(NULL);
  QT_CHECK(enc != NULL, "JxlEncoderCreate");
  JxlBasicInfo info;
  JxlEncoderInitBasicInfo(&info);
  info.xsize = W; info.ysize = H;
  info.bits_per_sample = 8; info.num_color_channels = 3;
  info.num_extra_channels = 1; info.alpha_bits = 8;
  info.uses_original_profile = JXL_TRUE;
  QT_CHECK(JxlEncoderSetBasicInfo(enc, &info) == JXL_ENC_SUCCESS, "SetBasicInfo");
  JxlColorEncoding color;
  JxlColorEncodingSetToSRGB(&color, JXL_FALSE);
  QT_CHECK(JxlEncoderSetColorEncoding(enc, &color) == JXL_ENC_SUCCESS, "SetColorEncoding");
  JxlEncoderFrameSettings *fs = JxlEncoderFrameSettingsCreate(enc, NULL);
  QT_CHECK(JxlEncoderSetFrameLossless(fs, JXL_TRUE) == JXL_ENC_SUCCESS, "SetFrameLossless");
  JxlPixelFormat fmt = { 4, JXL_TYPE_UINT8, JXL_NATIVE_ENDIAN, 0 };
  QT_CHECK(JxlEncoderAddImageFrame(fs, &fmt, img, sizeof img) == JXL_ENC_SUCCESS,
           "JxlEncoderAddImageFrame");
  JxlEncoderCloseInput(enc);
  size_t cap = sizeof img + 4096;
  uint8_t *out = malloc(cap);
  uint8_t *next = out; size_t avail = cap;
  JxlEncoderStatus st;
  do {
    st = JxlEncoderProcessOutput(enc, &next, &avail);
    if (st == JXL_ENC_NEED_MORE_OUTPUT) {
      size_t used = (size_t)(next - out);
      cap *= 2; out = realloc(out, cap);
      next = out + used; avail = cap - used;
    }
  } while (st == JXL_ENC_NEED_MORE_OUTPUT);
  QT_CHECK(st == JXL_ENC_SUCCESS, "JxlEncoderProcessOutput");
  size_t clen = (size_t)(next - out);
  QT_CHECK(clen > 0, "codestream bytes produced");
  QT_CHECK(JxlSignatureCheck(out, clen) != JXL_SIG_INVALID, "output carries a JXL signature");
  JxlEncoderDestroy(enc);

  JxlDecoder *dec = JxlDecoderCreate(NULL);
  QT_CHECK(dec != NULL, "JxlDecoderCreate");
  QT_CHECK(JxlDecoderSubscribeEvents(dec, JXL_DEC_BASIC_INFO | JXL_DEC_FULL_IMAGE)
           == JXL_DEC_SUCCESS, "SubscribeEvents");
  JxlDecoderSetInput(dec, out, clen);
  JxlDecoderCloseInput(dec);
  uint8_t back[H][W][4];
  int got_info = 0, got_image = 0;
  for (;;) {
    JxlDecoderStatus s = JxlDecoderProcessInput(dec);
    if (s == JXL_DEC_BASIC_INFO) {
      JxlBasicInfo bi;
      if (JxlDecoderGetBasicInfo(dec, &bi) == JXL_DEC_SUCCESS &&
          bi.xsize == W && bi.ysize == H) got_info = 1;
    } else if (s == JXL_DEC_NEED_IMAGE_OUT_BUFFER) {
      JxlDecoderSetImageOutBuffer(dec, &fmt, back, sizeof back);
    } else if (s == JXL_DEC_FULL_IMAGE) {
      got_image = 1;
    } else if (s == JXL_DEC_SUCCESS || s == JXL_DEC_ERROR) {
      break;
    }
  }
  QT_CHECK(got_info, "basic info decoded with the right size");
  QT_CHECK(got_image, "full image decoded");
  QT_CHECK(memcmp(img, back, sizeof img) == 0, "lossless round trip is byte-identical");
  JxlDecoderDestroy(dec);
  free(out);
  QT_END();
}
