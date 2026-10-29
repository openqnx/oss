/* libsamplerate: resample a tone 2:1 and check the output length and energy. */
#include "common.h"
#include <stdlib.h>
#include <math.h>
#include <samplerate.h>
#define N 4096
int main(void) {
  QT_BEGIN("libsamplerate");
  printf("  info %s\n", src_get_version());
  float in[N], out[N];
  for (int i = 0; i < N; ++i)
    in[i] = 0.5f * sinf(2.0f * (float)M_PI * 440.0f * i / 44100.0f);
  SRC_DATA d;
  memset(&d, 0, sizeof d);
  d.data_in = in; d.input_frames = N;
  d.data_out = out; d.output_frames = N;
  d.src_ratio = 0.5;                       /* 44100 -> 22050 */
  int err = src_simple(&d, SRC_SINC_FASTEST, 1);
  QT_CHECK(err == 0, "src_simple");
  if (err) printf("  info %s\n", src_strerror(err));
  QT_CHECK(d.output_frames_gen > N / 2 - 64 && d.output_frames_gen <= N / 2,
           "output length is about half the input");
  double e_in = 0, e_out = 0;
  for (int i = 0; i < N; ++i) e_in += in[i] * in[i];
  for (long i = 0; i < d.output_frames_gen; ++i) e_out += out[i] * out[i];
  QT_CHECK(fabs((e_out / d.output_frames_gen) / (e_in / N) - 1.0) < 0.15,
           "mean energy per sample is preserved");
  QT_CHECK(src_is_valid_ratio(0.5) && !src_is_valid_ratio(0.0), "src_is_valid_ratio");
  int e2 = 0;
  SRC_STATE *st = src_new(SRC_LINEAR, 1, &e2);
  QT_CHECK(st != NULL && e2 == 0, "src_new streaming state");
  src_delete(st);
  QT_END();
}
