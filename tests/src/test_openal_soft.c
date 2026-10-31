/* OpenAL Soft without audio hardware:
 *  - ALC_SOFT_loopback: render a 1 kHz mono buffer into 44.1 kHz stereo
 *    samples we read back -- zero crossings give the frequency, AL_GAIN 0.5
 *    halves the peak, a centred source is identical on both channels;
 *  - the "wave" backend (ALSOFT_CONF selects it): a real playback device that
 *    writes a WAV file, checked for its header and non-silent data;
 *  - openal-info against the same configuration. */
#include "common.h"
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include <unistd.h>
#include <time.h>
#include <AL/al.h>
#include <AL/alc.h>
#include <AL/alext.h>

static char dir[64];
static void msleep(int ms) { struct timespec ts = { ms / 1000, (ms % 1000) * 1000000L }; nanosleep(&ts, NULL); }

static ALuint make_tone(int rate, int n, double freq) {
  int16_t *s = malloc(n * sizeof *s);
  for (int i = 0; i < n; i++) s[i] = (int16_t)lrint(20000.0 * sin(2 * M_PI * freq * i / rate));
  ALuint b = 0; alGenBuffers(1, &b);
  alBufferData(b, AL_FORMAT_MONO16, s, n * (ALsizei)sizeof *s, rate);
  free(s); return b;
}

int main(void) {
  QT_BEGIN("openal-soft");
  strcpy(dir, "/tmp/qnxport-openal.XXXXXX");
  QT_CHECK(mkdtemp(dir) != NULL, "scratch directory");
  char conf[128], wav[128];
  snprintf(conf, sizeof conf, "%s/alsoft.conf", dir);
  snprintf(wav, sizeof wav, "%s/out.wav", dir);
  FILE *f = fopen(conf, "w");
  if (f) { fprintf(f, "[general]\ndrivers = wave\nfrequency = 22050\nchannels = stereo\nsample-type = int16\nstereo-encoding = panpot\n"
                      "[wave]\nfile = %s\n", wav); fclose(f); }
  setenv("ALSOFT_CONF", conf, 1);
  setenv("ALSOFT_LOGLEVEL", "1", 1);

  /* ---- loopback ---- */
  QT_CHECK(alcIsExtensionPresent(NULL, "ALC_SOFT_loopback"), "ALC_SOFT_loopback present");
  LPALCLOOPBACKOPENDEVICESOFT openLoop = (LPALCLOOPBACKOPENDEVICESOFT)alcGetProcAddress(NULL, "alcLoopbackOpenDeviceSOFT");
  LPALCISRENDERFORMATSUPPORTEDSOFT isFmt = (LPALCISRENDERFORMATSUPPORTEDSOFT)alcGetProcAddress(NULL, "alcIsRenderFormatSupportedSOFT");
  LPALCRENDERSAMPLESSOFT render = (LPALCRENDERSAMPLESSOFT)alcGetProcAddress(NULL, "alcRenderSamplesSOFT");
  QT_CHECK(openLoop && isFmt && render, "loopback entry points");
  ALCdevice *ld = openLoop ? openLoop(NULL) : NULL;
  QT_CHECK(ld != NULL, "alcLoopbackOpenDeviceSOFT");
  if (ld && isFmt && render) {
    QT_CHECK(isFmt(ld, 44100, ALC_STEREO_SOFT, ALC_SHORT_SOFT), "44.1 kHz stereo int16 render format supported");
    ALCint attrs[] = { ALC_FORMAT_CHANNELS_SOFT, ALC_STEREO_SOFT, ALC_FORMAT_TYPE_SOFT, ALC_SHORT_SOFT,
                       ALC_FREQUENCY, 44100, ALC_HRTF_SOFT, ALC_FALSE, 0 };
    ALCcontext *ctx = alcCreateContext(ld, attrs);
    QT_CHECK(ctx && alcMakeContextCurrent(ctx), "loopback context");
    printf("  info AL_VERSION %s, AL_RENDERER %s\n", alGetString(AL_VERSION) ? alGetString(AL_VERSION) : "?",
           alGetString(AL_RENDERER) ? alGetString(AL_RENDERER) : "?");
    ALuint buf = make_tone(44100, 44100, 1000.0), src = 0;
    alGenSources(1, &src);
    alSourcei(src, AL_BUFFER, (ALint)buf);
    alSourcei(src, AL_LOOPING, AL_TRUE);
    alSourcei(src, AL_SOURCE_RELATIVE, AL_TRUE);
    alSource3f(src, AL_POSITION, 0, 0, 0);
    alSourcePlay(src);
    QT_CHECK(alGetError() == AL_NO_ERROR, "buffer, source, play: no AL error");
    enum { N = 8820 };  /* 0.2 s */
    static int16_t out[N * 2];
    render(ld, out, 1024);            /* let the mixer settle */
    render(ld, out, N);
    int peak = 0, zc = 0, same = 1;
    for (int i = 0; i < N; i++) {
      int l = out[2 * i], r = out[2 * i + 1];
      if (abs(l) > peak) peak = abs(l);
      if (abs(l - r) > 2) same = 0;
      if (i && ((out[2 * (i - 1)] < 0) != (l < 0))) zc++;
    }
    printf("  info loopback: peak %d, %d zero crossings in 0.2 s\n", peak, zc);
    QT_CHECK(peak > 1000, "rendered output is not silent");
    QT_CHECK(zc >= 396 && zc <= 404, "1 kHz tone: ~400 zero crossings in 0.2 s");
    QT_CHECK(same, "centred source: left == right");
    alSourcef(src, AL_GAIN, 0.5f);
    render(ld, out, 4096);            /* gain changes ramp in */
    render(ld, out, N);
    int peak2 = 0;
    for (int i = 0; i < N * 2; i++) if (abs(out[i]) > peak2) peak2 = abs(out[i]);
    printf("  info AL_GAIN 0.5: peak %d (was %d)\n", peak2, peak);
    QT_CHECK(peak2 > peak * 0.45 && peak2 < peak * 0.55, "AL_GAIN 0.5 halves the amplitude");
    alSourceStop(src);
    alDeleteSources(1, &src); alDeleteBuffers(1, &buf);
    alcMakeContextCurrent(NULL); alcDestroyContext(ctx);
    alcCloseDevice(ld);
  }

  /* ---- the wave file writer as an ordinary playback device ---- */
  const ALCchar *def = alcGetString(NULL, ALC_DEFAULT_DEVICE_SPECIFIER);
  printf("  info default device: %s\n", def ? def : "(null)");
  ALCdevice *dev = alcOpenDevice(NULL);
  QT_CHECK(dev != NULL, "alcOpenDevice(NULL) with drivers = wave");
  if (dev) {
    const ALCchar *nm = alcGetString(dev, ALC_ALL_DEVICES_SPECIFIER);
    printf("  info opened: %s\n", nm ? nm : "(null)");
    QT_CHECK(nm && strstr(nm, "Wave File Writer"), "device is the Wave File Writer");
    ALCcontext *ctx = alcCreateContext(dev, NULL);
    alcMakeContextCurrent(ctx);
    ALCint fr = 0; alcGetIntegerv(dev, ALC_FREQUENCY, 1, &fr);
    printf("  info device frequency %d\n", fr);
    QT_CHECK(fr == 22050, "configured frequency 22050 applied");
    ALuint buf = make_tone(22050, 11025, 441.0), src = 0;
    alGenSources(1, &src); alSourcei(src, AL_BUFFER, (ALint)buf); alSourcePlay(src);
    ALint state = AL_PLAYING; int waited = 0;
    while (state == AL_PLAYING && waited < 5000) { msleep(50); waited += 50; alGetSourcei(src, AL_SOURCE_STATE, &state); }
    printf("  info source stopped after %d ms\n", waited);
    QT_CHECK(state == AL_STOPPED, "0.5 s buffer played to the end");
    alDeleteSources(1, &src); alDeleteBuffers(1, &buf);
    alcMakeContextCurrent(NULL); alcDestroyContext(ctx);
    alcCloseDevice(dev);
    f = fopen(wav, "rb");
    unsigned char h[44] = {0}; long nz = 0, n = 0;
    if (f) {
      if (fread(h, 1, 44, f) == 44) {
        int16_t s; while (fread(&s, 2, 1, f) == 1) { n++; if (abs(s) > 1000) nz++; }
      }
      fclose(f);
    }
    uint32_t rate = h[24] | h[25] << 8 | h[26] << 16 | (uint32_t)h[27] << 24;
    printf("  info out.wav: %u Hz, %u ch, %ld samples, %ld loud\n", rate, h[22], n, nz);
    QT_CHECK(!memcmp(h, "RIFF", 4) && !memcmp(h + 8, "WAVE", 4), "wave backend wrote RIFF/WAVE");
    QT_CHECK(rate == 22050 && h[22] == 2, "WAV header: 22050 Hz stereo");
    QT_CHECK(nz > 11025, "the tone is in the file (>0.25 s of loud stereo samples)");
  }

  /* openal-info searches the working directory for HRTF data through
     std::filesystem::current_path(), which libc++ sizes with pathconf(). */
  { char cwd[512]; errno = 0; long pc = pathconf(".", _PC_PATH_MAX); int e = errno;
    const char *c = getcwd(cwd, sizeof cwd);
    printf("  info cwd %s, pathconf(\".\", _PC_PATH_MAX) = %ld (%s)\n", c ? c : "(unknown)", pc, pc < 0 ? strerror(e) : "ok"); }
  char cmd[256];
  snprintf(cmd, sizeof cmd, "openal-info > %s/info.txt 2>&1", dir);
  int rc = system(cmd);
  printf("  info $ openal-info -> %d\n", rc);
  char txt[16384] = {0};
  snprintf(cmd, sizeof cmd, "%s/info.txt", dir);
  f = fopen(cmd, "r");
  if (f) { size_t k = fread(txt, 1, sizeof txt - 1, f); txt[k] = 0; fclose(f); }
  if (rc != 0) {   /* say why: the last lines openal-info printed */
    size_t len = strlen(txt), from = len > 1500 ? len - 1500 : 0;
    printf("  info openal-info output (tail):\n%s\n", txt + from);
  }
  QT_CHECK(rc == 0, "openal-info exits 0");
  QT_CHECK(strstr(txt, "OpenAL vendor string: OpenAL Community") != NULL, "openal-info: vendor OpenAL Community");
  QT_CHECK(strstr(txt, "Wave File Writer") != NULL, "openal-info lists the Wave File Writer");
  QT_CHECK(strstr(txt, "ALC_SOFT_loopback") != NULL, "openal-info lists ALC_SOFT_loopback");
  snprintf(cmd, sizeof cmd, "rm -rf %s", dir); system(cmd);
  QT_END();
}
