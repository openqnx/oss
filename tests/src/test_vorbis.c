/* libogg + libvorbis: encode a tone into an Ogg Vorbis stream in memory and
 * decode it back, which exercises both the container and the codec. */
#include "common.h"
#include <stdlib.h>
#include <math.h>
#include <ogg/ogg.h>
#include <vorbis/codec.h>
#include <vorbis/vorbisenc.h>
#define RATE 44100
#define N 8192
static unsigned char stream[262144];
static size_t stream_n;
static void emit(ogg_page *og) {
  memcpy(stream + stream_n, og->header, (size_t)og->header_len); stream_n += (size_t)og->header_len;
  memcpy(stream + stream_n, og->body, (size_t)og->body_len);     stream_n += (size_t)og->body_len;
}
int main(void) {
  QT_BEGIN("ogg/vorbis");
  printf("  info %s\n", vorbis_version_string());
  vorbis_info vi; vorbis_info_init(&vi);
  QT_CHECK(vorbis_encode_init_vbr(&vi, 1, RATE, 0.4f) == 0, "vorbis_encode_init_vbr");
  vorbis_comment vc; vorbis_comment_init(&vc);
  vorbis_comment_add_tag(&vc, "ENCODER", "qnx-port-test");
  vorbis_dsp_state vd; vorbis_block vb;
  QT_CHECK(vorbis_analysis_init(&vd, &vi) == 0, "vorbis_analysis_init");
  vorbis_block_init(&vd, &vb);
  ogg_stream_state os;
  ogg_stream_init(&os, 0x5150);
  ogg_packet h, hc, hb;
  QT_CHECK(vorbis_analysis_headerout(&vd, &vc, &h, &hc, &hb) == 0, "headerout");
  ogg_stream_packetin(&os, &h);
  ogg_stream_packetin(&os, &hc);
  ogg_stream_packetin(&os, &hb);
  ogg_page og;
  while (ogg_stream_flush(&os, &og)) emit(&og);
  QT_CHECK(stream_n > 0, "header pages written");

  float **buf = vorbis_analysis_buffer(&vd, N);
  for (int i = 0; i < N; ++i)
    buf[0][i] = 0.5f * sinf(2.0f * (float)M_PI * 440.0f * i / RATE);
  vorbis_analysis_wrote(&vd, N);
  vorbis_analysis_wrote(&vd, 0);
  ogg_packet op;
  while (vorbis_analysis_blockout(&vd, &vb) == 1) {
    vorbis_analysis(&vb, NULL);
    vorbis_bitrate_addblock(&vb);
    while (vorbis_bitrate_flushpacket(&vd, &op)) {
      ogg_stream_packetin(&os, &op);
      while (ogg_stream_pageout(&os, &og)) emit(&og);
    }
  }
  while (ogg_stream_flush(&os, &og)) emit(&og);
  QT_CHECK(stream_n > 1000, "audio pages written");
  ogg_stream_clear(&os); vorbis_block_clear(&vb); vorbis_dsp_clear(&vd);
  vorbis_comment_clear(&vc); vorbis_info_clear(&vi);

  /* Decode the bytes back. */
  ogg_sync_state oy; ogg_sync_init(&oy);
  char *dst = ogg_sync_buffer(&oy, (long)stream_n);
  memcpy(dst, stream, stream_n);
  ogg_sync_wrote(&oy, (long)stream_n);
  QT_CHECK(ogg_sync_pageout(&oy, &og) == 1, "first page recovered");
  ogg_stream_state dos; ogg_stream_init(&dos, ogg_page_serialno(&og));
  vorbis_info dvi; vorbis_info_init(&dvi);
  vorbis_comment dvc; vorbis_comment_init(&dvc);
  ogg_stream_pagein(&dos, &og);
  int headers = 0;
  while (headers < 3) {
    if (ogg_stream_packetout(&dos, &op) == 1) {
      if (vorbis_synthesis_headerin(&dvi, &dvc, &op) == 0) ++headers; else break;
    } else if (ogg_sync_pageout(&oy, &og) == 1) {
      ogg_stream_pagein(&dos, &og);
    } else break;
  }
  QT_CHECK(headers == 3, "three Vorbis headers parsed");
  QT_CHECK(dvi.rate == RATE && dvi.channels == 1, "decoded stream parameters match");
  const char *tag = vorbis_comment_query(&dvc, "ENCODER", 0);
  QT_CHECK(tag && strcmp(tag, "qnx-port-test") == 0, "comment tag survives");
  ogg_stream_clear(&dos); vorbis_comment_clear(&dvc); vorbis_info_clear(&dvi);
  ogg_sync_clear(&oy);
  QT_END();
}
