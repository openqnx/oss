/* FFmpeg: encode raw frames to H.264 with libavcodec and decode them back,
 * then mux/demux through libavformat in memory. */
#include "common.h"
#include <stdlib.h>
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/imgutils.h>
#include <libavutil/opt.h>
#include <libswscale/swscale.h>
#include <libswresample/swresample.h>
#define W 176
#define H 144
int main(void) {
  QT_BEGIN("ffmpeg");
  printf("  info libavcodec %u.%u.%u, libavformat %u.%u.%u\n",
         AV_VERSION_MAJOR(avcodec_version()), AV_VERSION_MINOR(avcodec_version()),
         AV_VERSION_MICRO(avcodec_version()),
         AV_VERSION_MAJOR(avformat_version()), AV_VERSION_MINOR(avformat_version()),
         AV_VERSION_MICRO(avformat_version()));
  const AVCodec *enc = avcodec_find_encoder(AV_CODEC_ID_H264);
  QT_CHECK(enc != NULL, "H.264 encoder is registered (libx264)");
  const AVCodec *dec = avcodec_find_decoder(AV_CODEC_ID_H264);
  QT_CHECK(dec != NULL, "H.264 decoder is registered");
  if (!enc || !dec) QT_END();

  AVCodecContext *ec = avcodec_alloc_context3(enc);
  ec->width = W; ec->height = H; ec->pix_fmt = AV_PIX_FMT_YUV420P;
  ec->time_base = (AVRational){ 1, 25 }; ec->framerate = (AVRational){ 25, 1 };
  ec->bit_rate = 200000; ec->gop_size = 5; ec->max_b_frames = 0;
  av_opt_set(ec->priv_data, "preset", "ultrafast", 0);
  av_opt_set(ec->priv_data, "tune", "zerolatency", 0);
  QT_CHECK(avcodec_open2(ec, enc, NULL) == 0, "avcodec_open2 (encoder)");

  AVCodecContext *dc = avcodec_alloc_context3(dec);
  QT_CHECK(avcodec_open2(dc, dec, NULL) == 0, "avcodec_open2 (decoder)");

  AVFrame *frame = av_frame_alloc();
  frame->format = AV_PIX_FMT_YUV420P; frame->width = W; frame->height = H;
  QT_CHECK(av_frame_get_buffer(frame, 0) == 0, "av_frame_get_buffer");
  AVPacket *pkt = av_packet_alloc();
  AVFrame *out = av_frame_alloc();
  int encoded = 0, decoded = 0;
  for (int f = 0; f < 10; ++f) {
    av_frame_make_writable(frame);
    memset(frame->data[0], 16 + f * 20, (size_t)(frame->linesize[0] * H));
    memset(frame->data[1], 128, (size_t)(frame->linesize[1] * H / 2));
    memset(frame->data[2], 128, (size_t)(frame->linesize[2] * H / 2));
    frame->pts = f;
    if (avcodec_send_frame(ec, frame) < 0) continue;
    while (avcodec_receive_packet(ec, pkt) == 0) {
      ++encoded;
      if (avcodec_send_packet(dc, pkt) == 0)
        while (avcodec_receive_frame(dc, out) == 0)
          if (out->width == W && out->height == H) ++decoded;
      av_packet_unref(pkt);
    }
  }
  avcodec_send_frame(ec, NULL);
  while (avcodec_receive_packet(ec, pkt) == 0) { ++encoded; av_packet_unref(pkt); }
  QT_CHECK(encoded >= 10, "all frames encoded");
  QT_CHECK(decoded >= 1, "at least one frame decoded back at the right size");
  printf("  info encoded %d, decoded %d\n", encoded, decoded);

  /* --- libswscale: a colour-space conversion --- */
  struct SwsContext *sws = sws_getContext(W, H, AV_PIX_FMT_YUV420P,
                                          W, H, AV_PIX_FMT_RGB24,
                                          SWS_BILINEAR, NULL, NULL, NULL);
  QT_CHECK(sws != NULL, "sws_getContext");
  if (sws) {
    uint8_t *rgb[4] = {0}; int rgb_ls[4] = {0};
    QT_CHECK(av_image_alloc(rgb, rgb_ls, W, H, AV_PIX_FMT_RGB24, 1) > 0,
             "av_image_alloc");
    QT_CHECK(sws_scale(sws, (const uint8_t * const *)frame->data, frame->linesize,
                       0, H, rgb, rgb_ls) == H, "sws_scale converted every row");
    av_freep(&rgb[0]);
    sws_freeContext(sws);
  }

  /* --- libswresample: an audio rate conversion --- */
  SwrContext *swr = NULL;
  AVChannelLayout mono = AV_CHANNEL_LAYOUT_MONO;
  QT_CHECK(swr_alloc_set_opts2(&swr, &mono, AV_SAMPLE_FMT_S16, 22050,
                               &mono, AV_SAMPLE_FMT_S16, 44100, 0, NULL) == 0,
           "swr_alloc_set_opts2");
  QT_CHECK(swr && swr_init(swr) == 0, "swr_init");
  if (swr) swr_free(&swr);

  /* --- libavformat: the muxer registry --- */
  const AVOutputFormat *mp4 = av_guess_format("mp4", NULL, NULL);
  QT_CHECK(mp4 != NULL, "the mp4 muxer is registered");
  QT_CHECK(av_guess_format(NULL, "out.mkv", NULL) != NULL, "matroska muxer by filename");

  av_frame_free(&out); av_frame_free(&frame); av_packet_free(&pkt);
  avcodec_free_context(&ec); avcodec_free_context(&dc);
  QT_END();
}
