/* libtheora: encode a couple of frames and read the headers back. */
#include "common.h"
#include <stdlib.h>
#include <ogg/ogg.h>
#include <theora/theoraenc.h>
#include <theora/theoradec.h>
#define W 64
#define H 48
int main(void) {
  QT_BEGIN("theora");
  printf("  info %s\n", th_version_string());
  th_info ti; th_info_init(&ti);
  ti.frame_width = W; ti.frame_height = H;
  ti.pic_width = W; ti.pic_height = H; ti.pic_x = 0; ti.pic_y = 0;
  ti.colorspace = TH_CS_ITU_REC_470M; ti.pixel_fmt = TH_PF_420;
  ti.target_bitrate = 200000;
  ti.fps_numerator = 25; ti.fps_denominator = 1;
  th_enc_ctx *enc = th_encode_alloc(&ti);
  QT_CHECK(enc != NULL, "th_encode_alloc");
  th_comment tc; th_comment_init(&tc);
  th_comment_add_tag(&tc, (char *)"ENCODER", (char *)"qnx-port-test");
  ogg_packet op;
  int headers = 0;
  while (th_encode_flushheader(enc, &tc, &op) > 0) ++headers;
  QT_CHECK(headers == 3, "three Theora headers produced");

  unsigned char y[W * H], u[W * H / 4], v[W * H / 4];
  th_ycbcr_buffer buf;
  buf[0].width = W; buf[0].height = H; buf[0].stride = W; buf[0].data = y;
  buf[1].width = W / 2; buf[1].height = H / 2; buf[1].stride = W / 2; buf[1].data = u;
  buf[2].width = W / 2; buf[2].height = H / 2; buf[2].stride = W / 2; buf[2].data = v;
  int packets = 0;
  for (int f = 0; f < 3; ++f) {
    memset(y, 16 + f * 40, sizeof y);
    memset(u, 128, sizeof u); memset(v, 128, sizeof v);
    QT_CHECK(th_encode_ycbcr_in(enc, buf) == 0, "th_encode_ycbcr_in");
    while (th_encode_packetout(enc, f == 2, &op) > 0) ++packets;
  }
  QT_CHECK(packets >= 3, "one packet per frame");
  th_encode_free(enc);
  th_comment_clear(&tc); th_info_clear(&ti);
  QT_END();
}
