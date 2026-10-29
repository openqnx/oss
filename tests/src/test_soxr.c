/* libsoxr: high-quality sample rate conversion. */
#include "common.h"
#include <stdlib.h>
#include <math.h>
#include <soxr.h>
#define N 4096
int main(void) {
  QT_BEGIN("soxr");
  printf("  info %s\n", soxr_version());
  float in[N], out[N];
  for (int i = 0; i < N; ++i)
    in[i] = 0.5f * sinf(2.0f * (float)M_PI * 440.0f * i / 44100.0f);
  size_t idone = 0, odone = 0;
  soxr_error_t err = soxr_oneshot(44100, 22050, 1,
                                  in, N, &idone, out, N, &odone,
                                  NULL, NULL, NULL);
  QT_CHECK(err == NULL, "soxr_oneshot");
  if (err) printf("  info %s\n", err);
  QT_CHECK(idone == N, "all input consumed");
  QT_CHECK(odone > N / 2 - 64 && odone <= N / 2 + 8,
           "output length is about half the input");
  double e_in = 0, e_out = 0;
  for (int i = 0; i < N; ++i) e_in += in[i] * in[i];
  for (size_t i = 0; i < odone; ++i) e_out += out[i] * out[i];
  QT_CHECK(fabs((e_out / odone) / (e_in / N) - 1.0) < 0.15,
           "mean energy per sample is preserved");
  /* The streaming API, which is what a real pipeline uses. */
  soxr_error_t e2 = NULL;
  soxr_t s = soxr_create(48000, 44100, 2, &e2, NULL, NULL, NULL);
  QT_CHECK(s != NULL && e2 == NULL, "soxr_create (stereo 48k -> 44.1k)");
  if (s) {
    QT_CHECK(soxr_engine(s) != NULL, "soxr_engine names a resampler");
    printf("  info engine = %s\n", soxr_engine(s));
    soxr_delete(s);
  }
  QT_END();
}
