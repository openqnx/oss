/* libtheora: a full encode -> decode round trip. Four 64x48 frames with a
 * known luma ramp and solid chroma are encoded at high quality; the three
 * header packets set up a decoder that reports the same geometry, frame
 * rate and comment tag, and every decoded frame is close to its source
 * (mean absolute luma error and exact-ish chroma). */
#include "common.h"
#include <stdlib.h>
#include <ogg/ogg.h>
#include <theora/theoraenc.h>
#include <theora/theoradec.h>
#define W 64
#define H 48
#define NF 4

static ogg_packet copy(const ogg_packet *op) {
  ogg_packet c = *op;
  c.packet = malloc(op->bytes ? op->bytes : 1);
  memcpy(c.packet, op->packet, op->bytes);
  return c;
}
static unsigned char Y(int f, int x, int y) { return (unsigned char)(32 + x * 2 + y + f * 10); }

int main(void) {
  QT_BEGIN("libtheora");
  printf("  info %s (bitstream %08x)\n", th_version_string(), (unsigned)th_version_number());
  th_info ti; th_info_init(&ti);
  ti.frame_width = W; ti.frame_height = H; ti.pic_width = W; ti.pic_height = H;
  ti.colorspace = TH_CS_ITU_REC_470BG; ti.pixel_fmt = TH_PF_420;
  ti.target_bitrate = 0; ti.quality = 60;
  ti.fps_numerator = 30000; ti.fps_denominator = 1001;
  ti.aspect_numerator = 1; ti.aspect_denominator = 1;
  th_enc_ctx *enc = th_encode_alloc(&ti);
  QT_CHECK(enc != NULL, "encoder allocated");
  if (!enc) QT_END();
  th_comment tc; th_comment_init(&tc);
  th_comment_add_tag(&tc, "TITLE", "qnx round trip");
  ogg_packet hdr[3], pkt[NF], op;
  int nh = 0, np = 0;
  while (nh < 3 && th_encode_flushheader(enc, &tc, &op) > 0) hdr[nh++] = copy(&op);
  QT_CHECK(nh == 3 && th_packet_isheader(&hdr[0]) && th_packet_isheader(&hdr[2]), "three header packets");

  static unsigned char yp[W * H], up[W * H / 4], vp[W * H / 4];
  th_ycbcr_buffer in = {{W, H, W, yp}, {W / 2, H / 2, W / 2, up}, {W / 2, H / 2, W / 2, vp}};
  for (int f = 0; f < NF; ++f) {
    for (int y = 0; y < H; ++y) for (int x = 0; x < W; ++x) yp[y * W + x] = Y(f, x, y);
    memset(up, 100, sizeof up); memset(vp, 160, sizeof vp);
    th_encode_ycbcr_in(enc, in);
    while (np < NF && th_encode_packetout(enc, f == NF - 1, &op) > 0) pkt[np++] = copy(&op);
  }
  QT_CHECK(np == NF, "one data packet per frame");
  QT_CHECK(np > 0 && th_packet_iskeyframe(&pkt[0]) == 1, "first frame is a keyframe");
  th_encode_free(enc);

  th_info di; th_info_init(&di);
  th_comment dc; th_comment_init(&dc);
  th_setup_info *setup = NULL;
  int hr = 0;
  for (int i = 0; i < nh; ++i) hr = th_decode_headerin(&di, &dc, &setup, &hdr[i]);
  QT_CHECK(hr > 0 && setup != NULL, "decoder accepts the headers");
  QT_CHECK(di.pic_width == W && di.pic_height == H && di.fps_numerator == 30000 && di.fps_denominator == 1001 &&
           di.pixel_fmt == TH_PF_420, "decoded stream info matches");
  char *title = th_comment_query(&dc, "TITLE", 0);
  QT_CHECK(title && strcmp(title, "qnx round trip") == 0, "comment tag survives");

  th_dec_ctx *dec = th_decode_alloc(&di, setup);
  int frames = 0, close_ok = 1;
  for (int i = 0; i < np && dec; ++i) {
    ogg_int64_t gp = 0;
    if (th_decode_packetin(dec, &pkt[i], &gp) != 0) { close_ok = 0; continue; }
    th_ycbcr_buffer out;
    th_decode_ycbcr_out(dec, out);
    long err = 0;
    for (int y = 0; y < H; ++y) for (int x = 0; x < W; ++x) err += labs((long)out[0].data[y * out[0].stride + x] - Y(i, x, y));
    double mae = (double)err / (W * H);
    int cu = out[1].data[5 * out[1].stride + 5], cv = out[2].data[5 * out[2].stride + 5];
    printf("  info frame %d: luma MAE %.2f, chroma %d/%d\n", i, mae, cu, cv);
    if (mae > 3.0 || abs(cu - 100) > 3 || abs(cv - 160) > 3) close_ok = 0;
    ++frames;
  }
  QT_CHECK(dec != NULL && frames == NF, "all frames decoded");
  QT_CHECK(close_ok, "decoded pictures match the source (luma MAE <= 3, chroma +-3)");
  if (dec) th_decode_free(dec);
  th_setup_free(setup);
  th_comment_clear(&dc); th_info_clear(&di); th_comment_clear(&tc); th_info_clear(&ti);
  for (int i = 0; i < nh; ++i) free(hdr[i].packet);
  for (int i = 0; i < np; ++i) free(pkt[i].packet);
  QT_END();
}
