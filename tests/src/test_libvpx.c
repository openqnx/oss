/* libvpx: encode VP8 frames and decode them back. */
#include "common.h"
#include <stdlib.h>
#define VPX_CODEC_DISABLE_COMPAT 1
#include <vpx/vpx_encoder.h>
#include <vpx/vpx_decoder.h>
#include <vpx/vp8cx.h>
#include <vpx/vp8dx.h>
#define W 176
#define H 144
int main(void) {
  QT_BEGIN("libvpx");
  printf("  info %s\n", vpx_codec_version_str());
  vpx_codec_enc_cfg_t cfg;
  QT_CHECK(vpx_codec_enc_config_default(vpx_codec_vp8_cx(), &cfg, 0) == VPX_CODEC_OK,
           "enc_config_default");
  cfg.g_w = W; cfg.g_h = H; cfg.g_timebase.num = 1; cfg.g_timebase.den = 25;
  cfg.rc_target_bitrate = 200; cfg.g_threads = 1; cfg.g_lag_in_frames = 0;
  vpx_codec_ctx_t enc;
  QT_CHECK(vpx_codec_enc_init(&enc, vpx_codec_vp8_cx(), &cfg, 0) == VPX_CODEC_OK,
           "vpx_codec_enc_init");
  vpx_codec_ctx_t dec;
  QT_CHECK(vpx_codec_dec_init(&dec, vpx_codec_vp8_dx(), NULL, 0) == VPX_CODEC_OK,
           "vpx_codec_dec_init");
  vpx_image_t img;
  QT_CHECK(vpx_img_alloc(&img, VPX_IMG_FMT_I420, W, H, 1) != NULL, "vpx_img_alloc");
  int packets = 0, frames_out = 0;
  for (int f = 0; f < 5; ++f) {
    memset(img.planes[0], 16 + f * 30, (size_t)(img.stride[0] * H));
    memset(img.planes[1], 128, (size_t)(img.stride[1] * H / 2));
    memset(img.planes[2], 128, (size_t)(img.stride[2] * H / 2));
    QT_CHECK(vpx_codec_encode(&enc, &img, f, 1, 0, VPX_DL_REALTIME) == VPX_CODEC_OK,
             f == 0 ? "vpx_codec_encode" : "encode frame");
    vpx_codec_iter_t it = NULL;
    const vpx_codec_cx_pkt_t *pkt;
    while ((pkt = vpx_codec_get_cx_data(&enc, &it))) {
      if (pkt->kind != VPX_CODEC_CX_FRAME_PKT) continue;
      ++packets;
      if (vpx_codec_decode(&dec, pkt->data.frame.buf, (unsigned)pkt->data.frame.sz,
                           NULL, 0) == VPX_CODEC_OK) {
        vpx_codec_iter_t dit = NULL;
        vpx_image_t *out;
        while ((out = vpx_codec_get_frame(&dec, &dit))) {
          if (out->d_w == W && out->d_h == H) ++frames_out;
        }
      }
    }
  }
  QT_CHECK(packets >= 5, "compressed packets produced");
  QT_CHECK(frames_out >= 5, "frames decoded back at the right size");
  printf("  info %d packet(s), %d decoded frame(s)\n", packets, frames_out);
  vpx_img_free(&img);
  vpx_codec_destroy(&enc);
  vpx_codec_destroy(&dec);
  QT_END();
}
