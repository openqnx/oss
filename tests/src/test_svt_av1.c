/* SVT-AV1: configure and run the encoder over a few frames. */
#include "common.h"
#include <stdlib.h>
#include <EbSvtAv1.h>
#include <EbSvtAv1Enc.h>
#define W 128
#define H 96
int main(void) {
  QT_BEGIN("svt-av1");
  printf("  info SVT-AV1 %d.%d.%d\n", SVT_AV1_VERSION_MAJOR, SVT_AV1_VERSION_MINOR,
         SVT_AV1_VERSION_PATCHLEVEL);
  EbComponentType *handle = NULL;
  EbSvtAv1EncConfiguration cfg;
  memset(&cfg, 0, sizeof cfg);
  EbErrorType rc = svt_av1_enc_init_handle(&handle, NULL, &cfg);
  QT_CHECK(rc == EB_ErrorNone && handle, "svt_av1_enc_init_handle");
  if (rc != EB_ErrorNone) QT_END();
  cfg.source_width = W; cfg.source_height = H;
  cfg.frame_rate_numerator = 25; cfg.frame_rate_denominator = 1;
  cfg.encoder_bit_depth = 8;
  cfg.enc_mode = 12;                    /* fastest preset */
  cfg.rate_control_mode = 0;
  cfg.level_of_parallelism = 1;
  QT_CHECK(svt_av1_enc_set_parameter(handle, &cfg) == EB_ErrorNone,
           "svt_av1_enc_set_parameter");
  QT_CHECK(svt_av1_enc_init(handle) == EB_ErrorNone, "svt_av1_enc_init");
  EbBufferHeaderType *stream = NULL;
  QT_CHECK(svt_av1_enc_stream_header(handle, &stream) == EB_ErrorNone && stream,
           "svt_av1_enc_stream_header");
  if (stream) {
    QT_CHECK(stream->n_filled_len > 0, "sequence header bytes produced");
    svt_av1_enc_stream_header_release(stream);
  }
  unsigned char *y = malloc(W * H), *u = malloc(W * H / 4), *v = malloc(W * H / 4);
  EbSvtIOFormat pic;
  memset(&pic, 0, sizeof pic);
  pic.luma = y; pic.cb = u; pic.cr = v;
  pic.y_stride = W; pic.cb_stride = W / 2; pic.cr_stride = W / 2;
  pic.width = W; pic.height = H; pic.bit_depth = EB_EIGHT_BIT;
  pic.color_fmt = EB_YUV420;
  EbBufferHeaderType hdr;
  memset(&hdr, 0, sizeof hdr);
  hdr.p_buffer = (uint8_t *)&pic;
  hdr.size = sizeof hdr;
  /* The library checks n_filled_len against the frame size it computed from
   * the configuration; leaving it at zero makes it reject every picture. */
  hdr.n_filled_len = (uint32_t)(W * H * 3 / 2);
  hdr.n_alloc_len = hdr.n_filled_len;
  hdr.pic_type = EB_AV1_INVALID_PICTURE;
  int sent = 0;
  for (int f = 0; f < 3; ++f) {
    memset(y, 16 + f * 40, (size_t)(W * H));
    memset(u, 128, (size_t)(W * H / 4)); memset(v, 128, (size_t)(W * H / 4));
    hdr.pts = f;
    if (svt_av1_enc_send_picture(handle, &hdr) == EB_ErrorNone) ++sent;
  }
  QT_CHECK(sent == 3, "frames accepted by the encoder");
  EbBufferHeaderType eos;
  memset(&eos, 0, sizeof eos);
  eos.flags = EB_BUFFERFLAG_EOS;
  eos.pic_type = EB_AV1_INVALID_PICTURE;
  svt_av1_enc_send_picture(handle, &eos);
  int out_packets = 0;
  for (;;) {
    EbBufferHeaderType *pkt = NULL;
    EbErrorType r = svt_av1_enc_get_packet(handle, &pkt, 1);
    if (r != EB_ErrorNone || !pkt) break;
    int done = (pkt->flags & EB_BUFFERFLAG_EOS) != 0;
    if (pkt->n_filled_len > 0) ++out_packets;
    svt_av1_enc_release_out_buffer(&pkt);
    if (done) break;
  }
  QT_CHECK(out_packets >= 1, "encoded packets returned");
  printf("  info %d output packet(s)\n", out_packets);
  svt_av1_enc_deinit(handle);
  svt_av1_enc_deinit_handle(handle);
  free(y); free(u); free(v);
  QT_END();
}
