/* SpeexDSP: resample a sine from 48 kHz to 16 kHz. */
#include "common.h"
#include <speex/speex_resampler.h>
#include <math.h>
int main(void) {
  QT_BEGIN("speexdsp");
  int err; SpeexResamplerState *r = speex_resampler_init(1, 48000, 16000, 5, &err);
  QT_CHECK(r && err == RESAMPLER_ERR_SUCCESS, "resampler init");
  float in[4800], out[2000]; for (int i = 0; i < 4800; i++) in[i] = sinf(i * 0.05f);
  spx_uint32_t il = 4800, ol = 2000;
  QT_CHECK(speex_resampler_process_float(r, 0, in, &il, out, &ol) == 0, "process");
  QT_CHECK(ol >= 1500 && ol <= 1600, "output is a third of the input");
  speex_resampler_destroy(r);
  QT_END();
}
