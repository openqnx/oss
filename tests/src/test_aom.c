/* libaom: encode AV1 frames and decode them back with aom's own decoder. */
#include "common.h"
#include <stdlib.h>
#include <aom/aom_encoder.h>
#include <aom/aom_decoder.h>
#include <aom/aomcx.h>
#include <aom/aomdx.h>
#define W 128
#define H 96
int main(void) {
  QT_BEGIN("libaom");
  printf("  info %s\n", aom_codec_version_str());
  aom_codec_enc_cfg_t cfg;
  QT_CHECK(aom_codec_enc_config_default(aom_codec_av1_cx(), &cfg, 0) == AOM_CODEC_OK,
           "enc_config_default");
  cfg.g_w = W; cfg.g_h = H;
  cfg.g_timebase.num = 1; cfg.g_timebase.den = 25;
  cfg.rc_target_bitrate = 200; cfg.g_threads = 1; cfg.g_lag_in_frames = 0;
  cfg.g_usage = AOM_USAGE_REALTIME;
  aom_codec_ctx_t enc;
  QT_CHECK(aom_codec_enc_init(&enc, aom_codec_av1_cx(), &cfg, 0) == AOM_CODEC_OK,
           "aom_codec_enc_init");
  aom_codec_control(&enc, AOME_SET_CPUUSED, 9);
  aom_codec_ctx_t dec;
  QT_CHECK(aom_codec_dec_init(&dec, aom_codec_av1_dx(), NULL, 0) == AOM_CODEC_OK,
           "aom_codec_dec_init");
  aom_image_t img;
  QT_CHECK(aom_img_alloc(&img, AOM_IMG_FMT_I420, W, H, 1) != NULL, "aom_img_alloc");
  int packets = 0, decoded = 0;
  for (int f = 0; f < 3; ++f) {
    memset(img.planes[0], 16 + f * 40, (size_t)(img.stride[0] * H));
    memset(img.planes[1], 128, (size_t)(img.stride[1] * H / 2));
    memset(img.planes[2], 128, (size_t)(img.stride[2] * H / 2));
    QT_CHECK(aom_codec_encode(&enc, &img, f, 1, 0) == AOM_CODEC_OK,
             f == 0 ? "aom_codec_encode" : "encode frame");
    aom_codec_iter_t it = NULL;
    const aom_codec_cx_pkt_t *pkt;
    while ((pkt = aom_codec_get_cx_data(&enc, &it))) {
      if (pkt->kind != AOM_CODEC_CX_FRAME_PKT) continue;
      ++packets;
      if (aom_codec_decode(&dec, pkt->data.frame.buf, pkt->data.frame.sz, NULL) ==
          AOM_CODEC_OK) {
        aom_codec_iter_t dit = NULL;
        aom_image_t *out;
        while ((out = aom_codec_get_frame(&dec, &dit)))
          if (out->d_w == W && out->d_h == H) ++decoded;
      }
    }
  }
  aom_codec_encode(&enc, NULL, 0, 0, 0);       /* flush */
  aom_codec_iter_t it = NULL;
  const aom_codec_cx_pkt_t *pkt;
  while ((pkt = aom_codec_get_cx_data(&enc, &it)))
    if (pkt->kind == AOM_CODEC_CX_FRAME_PKT) ++packets;
  QT_CHECK(packets >= 3, "compressed packets produced");
  QT_CHECK(decoded >= 1, "at least one frame decoded back at the right size");
  printf("  info %d packet(s), %d decoded frame(s)\n", packets, decoded);
  aom_img_free(&img);
  aom_codec_destroy(&enc);
  aom_codec_destroy(&dec);
  QT_END();
}
