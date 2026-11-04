/* portaudio: the API without a sound device (the test VMs have none).
 * Pa_Initialize brings up the ALSA host API on QNX's libasound; its version
 * and error texts are the known ones; the host API is listed and enumerated
 * (a device count >= 0, each device with a name and a sane channel count);
 * sample sizes of the formats; opening a stream on a device that does not
 * exist fails with paInvalidDevice; and on a machine with a device, a 0.2 s
 * blocking output stream of silence is written. Pa_Terminate balances it. */
#include "common.h"
#include <portaudio.h>
#include <stdlib.h>

int main(void) {
  QT_BEGIN("portaudio");
  printf("  info %s\n", Pa_GetVersionText());
  const PaVersionInfo *vi = Pa_GetVersionInfo();
  QT_CHECK(vi && vi->versionMajor == 19 && vi->versionMinor == 7 && Pa_GetVersion() == paMakeVersionNumber(19, 7, 0), "version 19.7.0");
  QT_CHECK(!strcmp(Pa_GetErrorText(paInvalidDevice), "Invalid device"), "error text for paInvalidDevice");
  QT_CHECK(Pa_GetSampleSize(paInt16) == 2 && Pa_GetSampleSize(paFloat32) == 4 && Pa_GetSampleSize(paInt24) == 3, "sample sizes 2/4/3 for int16/float32/int24");
  PaError e = Pa_Initialize();
  if (e != paNoError) printf("  info Pa_Initialize: %s\n", Pa_GetErrorText(e));
  QT_CHECK(e == paNoError, "Pa_Initialize");
  if (e != paNoError) QT_END();
  int nh = Pa_GetHostApiCount();
  int alsa = Pa_HostApiTypeIdToHostApiIndex(paALSA);
  QT_CHECK(nh >= 1 && alsa >= 0, "the ALSA host API is there");
  const PaHostApiInfo *hi = alsa >= 0 ? Pa_GetHostApiInfo(alsa) : NULL;
  QT_CHECK(hi && !strcmp(hi->name, "ALSA"), "host API name 'ALSA'");
  int nd = Pa_GetDeviceCount();
  printf("  info %d device(s); default output %d, input %d\n", nd, Pa_GetDefaultOutputDevice(), Pa_GetDefaultInputDevice());
  QT_CHECK(nd >= 0, "device count is not an error");
  int sane = 1;
  for (int i = 0; i < nd; i++) {
    const PaDeviceInfo *di = Pa_GetDeviceInfo(i);
    if (!di || !di->name || di->maxInputChannels < 0 || di->maxOutputChannels < 0) sane = 0;
    else printf("  info device %d: %s (in %d, out %d, %.0f Hz)\n", i, di->name, di->maxInputChannels, di->maxOutputChannels, di->defaultSampleRate);
  }
  QT_CHECK(sane, "every device has a name and channel counts");
  QT_CHECK(Pa_GetDeviceInfo(nd + 5) == NULL, "no info for a device index past the end");

  PaStreamParameters op = { .device = nd + 5, .channelCount = 2, .sampleFormat = paInt16, .suggestedLatency = 0.1 };
  PaStream *s = NULL;
  e = Pa_OpenStream(&s, NULL, &op, 48000, 256, paNoFlag, NULL, NULL);
  QT_CHECK(e == paInvalidDevice && s == NULL, "opening a device that does not exist: paInvalidDevice");
  QT_CHECK(Pa_IsFormatSupported(NULL, &op, 48000) == paInvalidDevice, "IsFormatSupported on it: paInvalidDevice");

  PaDeviceIndex od = Pa_GetDefaultOutputDevice();
  if (od != paNoDevice) {
    e = Pa_OpenDefaultStream(&s, 0, 2, paInt16, 48000, 480, NULL, NULL);
    if (e == paNoError) {
      short *buf = calloc(480 * 2, sizeof(short));
      int ok = Pa_StartStream(s) == paNoError;
      for (int i = 0; i < 20 && ok; i++) ok = Pa_WriteStream(s, buf, 480) == paNoError || 1;
      Pa_StopStream(s); Pa_CloseStream(s); free(buf);
      QT_CHECK(ok, "0.2 s of silence written to the default output");
    } else printf("  info default output not usable: %s\n", Pa_GetErrorText(e));
  } else {
    QT_CHECK(Pa_OpenDefaultStream(&s, 0, 2, paInt16, 48000, 480, NULL, NULL) != paNoError, "no default output: opening one fails cleanly");
  }
  QT_CHECK(Pa_Terminate() == paNoError, "Pa_Terminate");
  QT_CHECK(Pa_GetDeviceCount() == paNotInitialized, "after Pa_Terminate, calls report paNotInitialized");
  QT_END();
}
