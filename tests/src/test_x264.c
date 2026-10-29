/* x264: encode a few frames of H.264 and check the NAL units come out. */
#include "common.h"
#include <stdlib.h>
#include <stdint.h>
#include <x264.h>
#define W 160
#define H 96
int main(void) {
  QT_BEGIN("x264");
  printf("  info x264 build %d\n", X264_BUILD);
  x264_param_t param;
  QT_CHECK(x264_param_default_preset(&param, "ultrafast", "zerolatency") == 0,
           "x264_param_default_preset");
  param.i_width = W; param.i_height = H;
  param.i_csp = X264_CSP_I420;
  param.i_fps_num = 25; param.i_fps_den = 1;
  param.i_threads = 1;
  param.i_log_level = X264_LOG_NONE;
  QT_CHECK(x264_param_apply_profile(&param, "baseline") == 0, "apply baseline profile");
  x264_t *enc = x264_encoder_open(&param);
  QT_CHECK(enc != NULL, "x264_encoder_open");
  if (!enc) QT_END();
  x264_picture_t pic, out;
  QT_CHECK(x264_picture_alloc(&pic, X264_CSP_I420, W, H) == 0, "x264_picture_alloc");
  x264_nal_t *nals; int nnal = 0, frames = 0, bytes = 0;
  QT_CHECK(x264_encoder_headers(enc, &nals, &nnal) >= 0 && nnal >= 2,
           "SPS/PPS headers emitted");
  for (int f = 0; f < 5; ++f) {
    memset(pic.img.plane[0], 16 + f * 30, (size_t)(W * H));
    memset(pic.img.plane[1], 128, (size_t)(W * H / 4));
    memset(pic.img.plane[2], 128, (size_t)(W * H / 4));
    pic.i_pts = f;
    int n = x264_encoder_encode(enc, &nals, &nnal, &pic, &out);
    if (n > 0) { ++frames; bytes += n; }
  }
  while (x264_encoder_delayed_frames(enc)) {
    int n = x264_encoder_encode(enc, &nals, &nnal, NULL, &out);
    if (n > 0) { ++frames; bytes += n; }
  }
  QT_CHECK(frames >= 5, "all frames encoded");
  QT_CHECK(bytes > 0, "bitstream bytes produced");
  printf("  info %d frame(s), %d byte(s)\n", frames, bytes);
  x264_picture_clean(&pic);
  x264_encoder_close(enc);
  QT_END();
}
