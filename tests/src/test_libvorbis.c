/* libvorbis: encode two seconds of a 1 kHz stereo sine (VBR, quality 0.4)
 * with vorbisenc into an Ogg file in /tmp, then decode it with vorbisfile
 * and check the stream info, the comment tag, the exact sample count, the
 * signal level and the tone's frequency (zero crossings). */
#include "common.h"
#include <math.h>
#include <stdlib.h>
#include <unistd.h>
#include <vorbis/vorbisenc.h>
#define OV_EXCLUDE_STATIC_CALLBACKS
#include <vorbis/vorbisfile.h>
#define RATE 44100
#define N (2 * RATE)
static int wpage(FILE *f, ogg_page *p) {
  return fwrite(p->header, 1, p->header_len, f) == (size_t)p->header_len &&
         fwrite(p->body, 1, p->body_len, f) == (size_t)p->body_len;
}
int main(void) {
  QT_BEGIN("libvorbis");
  printf("  info %s\n", vorbis_version_string());
  char path[64]; snprintf(path, sizeof path, "/tmp/qnxport-vorbis-%d.ogg", (int)getpid());
  FILE *f = fopen(path, "wb");
  QT_CHECK(f != NULL, "open output file in /tmp");
  if (!f) QT_END();
  vorbis_info vi; vorbis_info_init(&vi);
  QT_CHECK(vorbis_encode_init_vbr(&vi, 2, RATE, 0.4f) == 0, "vorbis_encode_init_vbr 2ch 44.1k q0.4");
  vorbis_comment vc; vorbis_comment_init(&vc); vorbis_comment_add_tag(&vc, "TITLE", "qnx sine");
  vorbis_dsp_state vd; vorbis_block vb; vorbis_analysis_init(&vd, &vi); vorbis_block_init(&vd, &vb);
  ogg_stream_state os; ogg_stream_init(&os, 1234);
  ogg_packet h0, h1, h2; ogg_page og; ogg_packet op;
  vorbis_analysis_headerout(&vd, &vc, &h0, &h1, &h2);
  ogg_stream_packetin(&os, &h0); ogg_stream_packetin(&os, &h1); ogg_stream_packetin(&os, &h2);
  int ok = 1;
  while (ogg_stream_flush(&os, &og)) ok &= wpage(f, &og);
  int done = 0;
  for (int pos = 0; !done; pos += 1024) {
    int n = N - pos < 1024 ? N - pos : 1024;
    if (n > 0) {
      float **buf = vorbis_analysis_buffer(&vd, n);
      for (int i = 0; i < n; i++) {
        float s = 0.5f * (float)sin(2 * M_PI * 1000.0 * (pos + i) / RATE);
        buf[0][i] = s; buf[1][i] = -s;
      }
      vorbis_analysis_wrote(&vd, n);
    } else vorbis_analysis_wrote(&vd, 0);   /* end of stream */
    while (vorbis_analysis_blockout(&vd, &vb) == 1) {
      vorbis_analysis(&vb, NULL); vorbis_bitrate_addblock(&vb);
      while (vorbis_bitrate_flushpacket(&vd, &op)) {
        ogg_stream_packetin(&os, &op);
        while (ogg_stream_pageout(&os, &og)) { ok &= wpage(f, &og); if (ogg_page_eos(&og)) done = 1; }
      }
    }
    if (n <= 0 && !done) { while (ogg_stream_flush(&os, &og)) ok &= wpage(f, &og); done = 1; }
  }
  while (ogg_stream_flush(&os, &og)) ok &= wpage(f, &og);
  long fsize = ftell(f); fclose(f);
  ogg_stream_clear(&os); vorbis_block_clear(&vb); vorbis_dsp_clear(&vd); vorbis_comment_clear(&vc); vorbis_info_clear(&vi);
  printf("  info encoded %d frames -> %ld bytes\n", N, fsize);
  QT_CHECK(ok && fsize > 2000 && fsize < 200000, "Ogg Vorbis file written (plausible size)");

  OggVorbis_File vf;
  int r = ov_fopen(path, &vf);
  QT_CHECK(r == 0, "ov_fopen");
  if (r == 0) {
    vorbis_info *di = ov_info(&vf, -1);
    QT_CHECK(di && di->channels == 2 && di->rate == RATE, "stream info: 2 channels, 44100 Hz");
    vorbis_comment *dc = ov_comment(&vf, -1);
    char *t = dc ? vorbis_comment_query(dc, "TITLE", 0) : NULL;
    QT_CHECK(t && strcmp(t, "qnx sine") == 0, "TITLE comment survives");
    ogg_int64_t tot = ov_pcm_total(&vf, -1);
    printf("  info ov_pcm_total = %lld, bitrate nominal %ld\n", (long long)tot, di ? di->bitrate_nominal : 0L);
    QT_CHECK(tot == N, "ov_pcm_total equals the encoded frame count");
    static short pcm[N * 2 + 8192];
    long got = 0; int bs = 0;
    while (got < (long)sizeof pcm) {
      long n = ov_read(&vf, (char *)pcm + got, (int)(sizeof pcm - got < 4096 ? sizeof pcm - got : 4096), 0, 2, 1, &bs);
      if (n <= 0) break;
      got += n;
    }
    long frames = got / 4;
    QT_CHECK(frames == N, "decoded frame count matches");
    /* steady-state region: skip the first and last 0.1 s */
    double sl = 0, sr = 0, corr = 0; int zc = 0;
    int a = RATE / 10, b = (int)(frames < N ? frames : N) - RATE / 10;
    for (int i = a; i < b; i++) {
      double L = pcm[2 * i] / 32768.0, R = pcm[2 * i + 1] / 32768.0;
      sl += L * L; sr += R * R; corr += L * R;
      if (i > a && (pcm[2 * (i - 1)] < 0) != (L < 0)) zc++;
    }
    double rmsl = sqrt(sl / (b - a)), rmsr = sqrt(sr / (b - a));
    double freq = zc / 2.0 / ((b - a) / (double)RATE);
    printf("  info rms L=%.4f R=%.4f (ideal 0.3536), est. frequency %.1f Hz, corr %.3f\n", rmsl, rmsr, freq,
           corr / sqrt(sl * sr + 1e-30));
    QT_CHECK(fabs(rmsl - 0.3536) < 0.05 && fabs(rmsr - 0.3536) < 0.05, "decoded level close to the input");
    QT_CHECK(fabs(freq - 1000.0) < 5.0, "decoded tone is 1 kHz");
    QT_CHECK(corr / sqrt(sl * sr + 1e-30) < -0.95, "right channel is the inverted left one");
    QT_CHECK(ov_pcm_seek(&vf, RATE) == 0 && ov_pcm_tell(&vf) == RATE, "ov_pcm_seek to 1 s");
    ov_clear(&vf);
  }
  unlink(path);
  QT_END();
}
