/* libsndfile: write a WAV and a FLAC file, read both back, compare. */
#include "common.h"
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include <sndfile.h>
#define N 2048
int main(void) {
  QT_BEGIN("libsndfile");
  char ver[64]; sf_command(NULL, SFC_GET_LIB_VERSION, ver, sizeof ver);
  printf("  info %s\n", ver);
  short pcm[N], back[N];
  for (int i = 0; i < N; ++i)
    pcm[i] = (short)(9000.0 * sin(2.0 * M_PI * 300.0 * i / 44100.0));
  struct { const char *ext; int fmt; const char *what; } cases[] = {
    { "wav",  SF_FORMAT_WAV  | SF_FORMAT_PCM_16, "WAV/PCM16" },
    { "flac", SF_FORMAT_FLAC | SF_FORMAT_PCM_16, "FLAC" },
  };
  for (size_t c = 0; c < sizeof cases / sizeof cases[0]; ++c) {
    char path[64];
    snprintf(path, sizeof path, "/tmp/qnxport-snd-%d.%s", (int)c, cases[c].ext);
    SF_INFO info; memset(&info, 0, sizeof info);
    info.samplerate = 44100; info.channels = 1; info.format = cases[c].fmt;
    SNDFILE *f = sf_open(path, SFM_WRITE, &info);
    QT_CHECK(f != NULL, cases[c].what);
    if (!f) { printf("  info %s\n", sf_strerror(NULL)); continue; }
    QT_CHECK(sf_write_short(f, pcm, N) == N, "sf_write_short");
    sf_close(f);
    memset(&info, 0, sizeof info);
    f = sf_open(path, SFM_READ, &info);
    QT_CHECK(f != NULL, "reopen for reading");
    QT_CHECK(info.frames == N && info.channels == 1 && info.samplerate == 44100,
             "header round trip");
    QT_CHECK(sf_read_short(f, back, N) == N, "sf_read_short");
    QT_CHECK(memcmp(pcm, back, sizeof pcm) == 0, "samples are bit-exact (lossless format)");
    sf_close(f);
    unlink(path);
  }
  QT_END();
}
