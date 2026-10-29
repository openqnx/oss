/* x265: encode a few frames of HEVC. */
#include "common.h"
#include <stdlib.h>
#include <x265.h>
#define W 160
#define H 96
int main(void) {
  QT_BEGIN("x265");
  printf("  info x265 %s (%d-bit)\n", x265_version_str, x265_max_bit_depth);
  const x265_api *api = x265_api_get(0);
  QT_CHECK(api != NULL, "x265_api_get");
  if (!api) QT_END();
  x265_param *param = api->param_alloc();
  QT_CHECK(param != NULL, "param_alloc");
  api->param_default_preset(param, "ultrafast", "zerolatency");
  param->sourceWidth = W; param->sourceHeight = H;
  param->fpsNum = 25; param->fpsDenom = 1;
  param->internalCsp = X265_CSP_I420;
  param->logLevel = X265_LOG_NONE;
  param->frameNumThreads = 1;
  param->bRepeatHeaders = 1;
  x265_encoder *enc = api->encoder_open(param);
  QT_CHECK(enc != NULL, "encoder_open");
  if (!enc) { api->param_free(param); QT_END(); }
  x265_picture *pic = api->picture_alloc();
  api->picture_init(param, pic);
  unsigned char *y = malloc(W * H), *u = malloc(W * H / 4), *v = malloc(W * H / 4);
  pic->planes[0] = y; pic->planes[1] = u; pic->planes[2] = v;
  pic->stride[0] = W; pic->stride[1] = W / 2; pic->stride[2] = W / 2;
  x265_nal *nals; uint32_t nnal = 0;
  int frames = 0; long bytes = 0;
  for (int f = 0; f < 5; ++f) {
    memset(y, 16 + f * 30, (size_t)(W * H));
    memset(u, 128, (size_t)(W * H / 4)); memset(v, 128, (size_t)(W * H / 4));
    pic->pts = f;
    int n = api->encoder_encode(enc, &nals, &nnal, pic, NULL);
    if (n > 0) { ++frames; for (uint32_t i = 0; i < nnal; ++i) bytes += nals[i].sizeBytes; }
  }
  while (api->encoder_encode(enc, &nals, &nnal, NULL, NULL) > 0) {
    ++frames; for (uint32_t i = 0; i < nnal; ++i) bytes += nals[i].sizeBytes;
  }
  QT_CHECK(frames >= 5, "all frames encoded");
  QT_CHECK(bytes > 0, "bitstream bytes produced");
  printf("  info %d frame(s), %ld byte(s)\n", frames, bytes);
  api->picture_free(pic);
  api->encoder_close(enc);
  api->param_free(param);
  free(y); free(u); free(v);
  QT_END();
}
