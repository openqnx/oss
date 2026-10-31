/* ffmpegthumbnailer: encode a 2 s, 64x48 MPEG-4 video whose left half is red
 * and right half blue (libavformat/libavcodec), then make thumbnails of it:
 *  - C API to an RGB buffer: requested size, aspect ratio, colours;
 *  - C API to a PNG file: signature and IHDR dimensions;
 *  - the ffmpegthumbnailer program to a JPEG, decoded with TurboJPEG. */
#include "common.h"
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libavutil/opt.h>
#include <libffmpegthumbnailer/videothumbnailerc.h>
#include <turbojpeg.h>

#define VW 64
#define VH 48
#define FPS 25
#define NFRAMES 50
static char dir[64];

static int encode(const char *path) {
  AVFormatContext *oc = NULL;
  if (avformat_alloc_output_context2(&oc, NULL, NULL, path) < 0 || !oc) return 0;
  const AVCodec *codec = avcodec_find_encoder(AV_CODEC_ID_MPEG4);
  if (!codec) { printf("  info no MPEG-4 encoder\n"); return 0; }
  AVStream *st = avformat_new_stream(oc, NULL);
  AVCodecContext *c = avcodec_alloc_context3(codec);
  c->width = VW; c->height = VH; c->pix_fmt = AV_PIX_FMT_YUV420P;
  c->time_base = (AVRational){ 1, FPS }; c->framerate = (AVRational){ FPS, 1 };
  c->gop_size = 5; c->bit_rate = 400000;
  if (oc->oformat->flags & AVFMT_GLOBALHEADER) c->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
  if (avcodec_open2(c, codec, NULL) < 0) return 0;
  avcodec_parameters_from_context(st->codecpar, c);
  st->time_base = c->time_base;
  if (avio_open(&oc->pb, path, AVIO_FLAG_WRITE) < 0) return 0;
  if (avformat_write_header(oc, NULL) < 0) return 0;
  AVFrame *fr = av_frame_alloc();
  fr->format = c->pix_fmt; fr->width = VW; fr->height = VH;
  av_frame_get_buffer(fr, 0);
  AVPacket *pkt = av_packet_alloc();
  int packets = 0;
  for (int i = 0; i <= NFRAMES; i++) {
    AVFrame *in = NULL;
    if (i < NFRAMES) {
      av_frame_make_writable(fr);
      for (int y = 0; y < VH; y++)
        for (int x = 0; x < VW; x++) fr->data[0][y * fr->linesize[0] + x] = x < VW / 2 ? 81 : 41;
      for (int y = 0; y < VH / 2; y++)
        for (int x = 0; x < VW / 2; x++) {
          fr->data[1][y * fr->linesize[1] + x] = x < VW / 4 ? 90 : 240;
          fr->data[2][y * fr->linesize[2] + x] = x < VW / 4 ? 240 : 110;
        }
      fr->pts = i; in = fr;
    }
    if (avcodec_send_frame(c, in) < 0) break;
    while (avcodec_receive_packet(c, pkt) == 0) {
      av_packet_rescale_ts(pkt, c->time_base, st->time_base);
      pkt->stream_index = st->index;
      av_interleaved_write_frame(oc, pkt);
      packets++;
    }
  }
  av_write_trailer(oc);
  avio_closep(&oc->pb);
  av_packet_free(&pkt); av_frame_free(&fr); avcodec_free_context(&c); avformat_free_context(oc);
  printf("  info encoded %d frames into %d packets\n", NFRAMES, packets);
  return packets == NFRAMES;
}
static int reddish(const uint8_t *p) { return p[0] > 180 && p[1] < 70 && p[2] < 70; }
static int bluish(const uint8_t *p) { return p[2] > 180 && p[0] < 70 && p[1] < 70; }

int main(void) {
  QT_BEGIN("ffmpegthumbnailer");
  strcpy(dir, "/tmp/qnxport-ffthumb.XXXXXX");
  QT_CHECK(mkdtemp(dir) != NULL, "scratch directory");
  char mp4[128]; snprintf(mp4, sizeof mp4, "%s/rb.mp4", dir);
  QT_CHECK(encode(mp4), "encoded a 2 s red|blue MPEG-4 video");

  video_thumbnailer *t = video_thumbnailer_create();
  QT_CHECK(t != NULL, "video_thumbnailer_create");
  image_data *img = video_thumbnailer_create_image_data();
  t->thumbnail_image_type = Rgb;
  video_thumbnailer_set_size(t, 32, 0);
  int rc = video_thumbnailer_generate_thumbnail_to_buffer(t, mp4, img);
  printf("  info RGB thumbnail rc=%d %dx%d, %d bytes\n", rc, img->image_data_width, img->image_data_height,
         img->image_data_size);
  QT_CHECK(rc == 0, "generate_thumbnail_to_buffer (RGB)");
  QT_CHECK(img->image_data_width == 32 && img->image_data_height == 24, "32 wide, aspect 4:3 kept -> 32x24");
  QT_CHECK(img->image_data_size == 32 * 24 * 3, "RGB buffer is width*height*3");
  if (rc == 0 && img->image_data_size == 32 * 24 * 3) {
    const uint8_t *l = img->image_data_ptr + (12 * 32 + 4) * 3, *r = img->image_data_ptr + (12 * 32 + 27) * 3;
    printf("  info left pixel %u,%u,%u  right pixel %u,%u,%u\n", l[0], l[1], l[2], r[0], r[1], r[2]);
    QT_CHECK(reddish(l), "left half is red");
    QT_CHECK(bluish(r), "right half is blue");
  }
  video_thumbnailer_destroy_image_data(img);

  t->thumbnail_image_type = Png;
  video_thumbnailer_set_size(t, 40, 30);
  char png[128]; snprintf(png, sizeof png, "%s/t.png", dir);
  QT_CHECK(video_thumbnailer_generate_thumbnail_to_file(t, mp4, png) == 0, "generate_thumbnail_to_file (PNG)");
  unsigned char h[24] = {0};
  FILE *f = fopen(png, "rb");
  if (f) { if (fread(h, 1, 24, f) != 24) h[0] = 0; fclose(f); }
  uint32_t pw = (uint32_t)h[16] << 24 | h[17] << 16 | h[18] << 8 | h[19];
  uint32_t ph = (uint32_t)h[20] << 24 | h[21] << 16 | h[22] << 8 | h[23];
  printf("  info PNG IHDR %ux%u\n", pw, ph);
  QT_CHECK(!memcmp(h, "\x89PNG\r\n\x1a\n", 8) && !memcmp(h + 12, "IHDR", 4), "PNG signature and IHDR");
  QT_CHECK(pw == 40 && ph == 30, "PNG is 40x30");
  video_thumbnailer_destroy(t);

  char cmd[512];
  snprintf(cmd, sizeof cmd, "ffmpegthumbnailer -i %s -o %s/cli.jpg -s 0 -q 10 -t 50%% > %s/cli.log 2>&1", mp4, dir, dir);
  rc = system(cmd);
  printf("  info $ ffmpegthumbnailer -i rb.mp4 -o cli.jpg -s 0 -q 10 -t 50%% -> %d\n", rc);
  QT_CHECK(rc == 0, "ffmpegthumbnailer program exits 0");
  snprintf(cmd, sizeof cmd, "%s/cli.jpg", dir);
  f = fopen(cmd, "rb");
  unsigned char *jb = NULL; long jn = 0;
  if (f) { fseek(f, 0, SEEK_END); jn = ftell(f); fseek(f, 0, SEEK_SET); jb = malloc(jn > 0 ? jn : 1);
           if (fread(jb, 1, jn, f) != (size_t)jn) jn = 0; fclose(f); }
  tjhandle td = tj3Init(TJINIT_DECOMPRESS);
  int ok = jb && jn > 0 && tj3DecompressHeader(td, jb, jn) == 0;
  QT_CHECK(ok, "program wrote a JPEG");
  if (ok) {
    int w = tj3Get(td, TJPARAM_JPEGWIDTH), hh = tj3Get(td, TJPARAM_JPEGHEIGHT);
    printf("  info JPEG %dx%d\n", w, hh);
    QT_CHECK(w == VW && hh == VH, "-s 0 keeps the original 64x48");
    if (w == VW && hh == VH) {
      static uint8_t px[VW * VH * 3];
      if (tj3Decompress8(td, jb, jn, px, 0, TJPF_RGB) == 0) {
        QT_CHECK(reddish(px + (24 * VW + 8) * 3) && bluish(px + (24 * VW + 56) * 3), "JPEG shows red | blue");
      } else QT_CHECK(0, "decode the program's JPEG");
    }
  }
  free(jb); tj3Destroy(td);
  snprintf(cmd, sizeof cmd, "rm -rf %s", dir); system(cmd);
  QT_END();
}
