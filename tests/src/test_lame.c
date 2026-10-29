/* LAME: encode PCM to MP3 and check the bitstream looks like MP3. */
#include "common.h"
#include <stdlib.h>
#include <math.h>
#include <lame/lame.h>
#define RATE 44100
#define N 8192
int main(void) {
  QT_BEGIN("lame");
  printf("  info LAME %s\n", get_lame_version());
  lame_global_flags *gf = lame_init();
  QT_CHECK(gf != NULL, "lame_init");
  lame_set_num_channels(gf, 1);
  lame_set_in_samplerate(gf, RATE);
  lame_set_brate(gf, 128);
  lame_set_mode(gf, MONO);
  lame_set_quality(gf, 5);
  QT_CHECK(lame_init_params(gf) >= 0, "lame_init_params");
  QT_CHECK(lame_get_out_samplerate(gf) == RATE, "output sample rate");
  short pcm[N];
  for (int i = 0; i < N; ++i)
    pcm[i] = (short)(10000.0 * sin(2.0 * M_PI * 440.0 * i / RATE));
  unsigned char mp3[N * 2];
  int n = lame_encode_buffer(gf, pcm, pcm, N, mp3, (int)sizeof mp3);
  QT_CHECK(n > 0, "lame_encode_buffer produced bytes");
  int tail = lame_encode_flush(gf, mp3 + n, (int)sizeof mp3 - n);
  QT_CHECK(tail >= 0, "lame_encode_flush");
  int total = n + tail;
  QT_CHECK(total < N * 2, "MP3 is smaller than the raw PCM");
  /* Every MPEG audio frame starts with an 11-bit sync word. */
  int sync = 0;
  for (int i = 0; i + 1 < total; ++i)
    if (mp3[i] == 0xFF && (mp3[i + 1] & 0xE0) == 0xE0) { sync = 1; break; }
  QT_CHECK(sync, "bitstream contains an MPEG frame sync word");
  lame_close(gf);
  QT_END();
}
