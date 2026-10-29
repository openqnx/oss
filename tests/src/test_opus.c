/* Opus: encode a tone and decode it back, checking the frame round-trips. */
#include "common.h"
#include <stdlib.h>
#include <math.h>
#include <opus/opus.h>
#define RATE 48000
#define FRAME 960          /* 20 ms */
int main(void) {
  QT_BEGIN("opus");
  printf("  info %s\n", opus_get_version_string());
  int err = 0;
  OpusEncoder *enc = opus_encoder_create(RATE, 1, OPUS_APPLICATION_AUDIO, &err);
  QT_CHECK(err == OPUS_OK && enc, "opus_encoder_create");
  OpusDecoder *dec = opus_decoder_create(RATE, 1, &err);
  QT_CHECK(err == OPUS_OK && dec, "opus_decoder_create");
  QT_CHECK(opus_encoder_ctl(enc, OPUS_SET_BITRATE(64000)) == OPUS_OK, "set bitrate");
  opus_int16 pcm[FRAME], back[FRAME];
  for (int i = 0; i < FRAME; ++i)
    pcm[i] = (opus_int16)(16000.0 * sin(2.0 * M_PI * 440.0 * i / RATE));
  unsigned char packet[4000];
  int n = opus_encode(enc, pcm, FRAME, packet, sizeof packet);
  QT_CHECK(n > 0, "opus_encode produced a packet");
  QT_CHECK(n < FRAME * 2, "packet is smaller than the raw PCM");
  QT_CHECK(opus_packet_get_nb_channels(packet) == 1, "packet says mono");
  QT_CHECK(opus_packet_get_samples_per_frame(packet, RATE) == FRAME,
           "packet says 960 samples per frame");
  int got = opus_decode(dec, packet, n, back, FRAME, 0);
  QT_CHECK(got == FRAME, "opus_decode returned a full frame");
  long energy = 0;
  for (int i = 0; i < FRAME; ++i) energy += labs(back[i]);
  QT_CHECK(energy / FRAME > 1000, "decoded frame carries the tone's energy");
  opus_encoder_destroy(enc);
  opus_decoder_destroy(dec);
  QT_END();
}
