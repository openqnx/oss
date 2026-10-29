/* FLAC: encode PCM to a file with the stream encoder, then decode it back and
 * verify the result is bit-exact -- FLAC is lossless, so anything else is a bug. */
#include "common.h"
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include <FLAC/stream_encoder.h>
#include <FLAC/stream_decoder.h>
#define N 4096
static FLAC__int32 orig[N];
static FLAC__int32 decoded[N];
static unsigned decoded_n;
static FLAC__StreamDecoderWriteStatus wcb(const FLAC__StreamDecoder *d,
    const FLAC__Frame *f, const FLAC__int32 *const buf[], void *client) {
  (void)d; (void)client;
  for (unsigned i = 0; i < f->header.blocksize && decoded_n < N; ++i)
    decoded[decoded_n++] = buf[0][i];
  return FLAC__STREAM_DECODER_WRITE_STATUS_CONTINUE;
}
static void ecb(const FLAC__StreamDecoder *d, FLAC__StreamDecoderErrorStatus s, void *c) {
  (void)d; (void)s; (void)c;
}
int main(void) {
  QT_BEGIN("flac");
  printf("  info FLAC %s\n", FLAC__VERSION_STRING);
  char path[] = "/tmp/qnxport-flac-XXXXXX";
  int fd = mkstemp(path);
  QT_CHECK(fd >= 0, "create temp file");
  if (fd >= 0) close(fd);
  for (int i = 0; i < N; ++i)
    orig[i] = (FLAC__int32)(8000.0 * sin(2.0 * M_PI * 330.0 * i / 44100.0));
  FLAC__StreamEncoder *enc = FLAC__stream_encoder_new();
  QT_CHECK(enc != NULL, "FLAC__stream_encoder_new");
  FLAC__stream_encoder_set_channels(enc, 1);
  FLAC__stream_encoder_set_bits_per_sample(enc, 16);
  FLAC__stream_encoder_set_sample_rate(enc, 44100);
  FLAC__stream_encoder_set_compression_level(enc, 5);
  QT_CHECK(FLAC__stream_encoder_init_file(enc, path, NULL, NULL) ==
           FLAC__STREAM_ENCODER_INIT_STATUS_OK, "init_file");
  const FLAC__int32 *chan[1] = { orig };
  QT_CHECK(FLAC__stream_encoder_process(enc, chan, N), "encode samples");
  QT_CHECK(FLAC__stream_encoder_finish(enc), "finish");
  FLAC__stream_encoder_delete(enc);

  FLAC__StreamDecoder *dec = FLAC__stream_decoder_new();
  QT_CHECK(dec != NULL, "FLAC__stream_decoder_new");
  QT_CHECK(FLAC__stream_decoder_init_file(dec, path, wcb, NULL, ecb, NULL) ==
           FLAC__STREAM_DECODER_INIT_STATUS_OK, "decoder init_file");
  QT_CHECK(FLAC__stream_decoder_process_until_end_of_stream(dec), "decode to end");
  FLAC__stream_decoder_finish(dec);
  FLAC__stream_decoder_delete(dec);
  QT_CHECK(decoded_n == N, "all samples decoded");
  QT_CHECK(memcmp(orig, decoded, sizeof orig) == 0, "FLAC round trip is bit-exact");
  unlink(path);
  QT_END();
}
