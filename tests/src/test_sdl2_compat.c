/* sdl2-compat: the SDL2 API on top of SDL3, headless.
 * The library is dlopen()ed, not linked: libSDL2-2.0.so.0 is installed by
 * both sdl2 and sdl2-compat, so a link-time dependency could name either
 * package. sdl2-compat itself dlopen()s libSDL3.so.0.
 *  - version 2.32.60, SDL_Init with the dummy video and the disk audio drivers
 *    (SDL2 environment names, translated to SDL3 hints by sdl2-compat);
 *  - surfaces: SDL_FillRect + a software renderer, pixels read back;
 *  - a window on the dummy driver with a window surface;
 *  - audio: an SDL2 callback device on the disk driver writes exactly the
 *    samples the callback produced to a file. */
#include "common.h"
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <dlfcn.h>
#include <SDL2/SDL.h>

#define SYM(name) static __typeof__(&name) p_##name;
#define LOAD(name) ok &= (p_##name = (__typeof__(&name))dlsym(h, #name)) != NULL; \
                   if (!p_##name) printf("  info missing symbol %s\n", #name);
SYM(SDL_GetVersion) SYM(SDL_Init) SYM(SDL_Quit) SYM(SDL_GetError) SYM(SDL_GetCurrentVideoDriver)
SYM(SDL_GetCurrentAudioDriver) SYM(SDL_CreateRGBSurfaceWithFormat) SYM(SDL_FreeSurface) SYM(SDL_FillRect)
SYM(SDL_MapRGB) SYM(SDL_CreateSoftwareRenderer) SYM(SDL_SetRenderDrawColor) SYM(SDL_RenderFillRect)
SYM(SDL_RenderPresent) SYM(SDL_RenderReadPixels) SYM(SDL_DestroyRenderer) SYM(SDL_CreateWindow)
SYM(SDL_GetWindowSurface) SYM(SDL_UpdateWindowSurface) SYM(SDL_DestroyWindow) SYM(SDL_GetWindowSize)
SYM(SDL_OpenAudioDevice) SYM(SDL_PauseAudioDevice) SYM(SDL_CloseAudioDevice) SYM(SDL_Delay)
SYM(SDL_GetTicks)

static int16_t produced[200000];
static volatile long nproduced = 0;
static void SDLCALL audio_cb(void *ud, Uint8 *stream, int len) {
  (void)ud;
  int16_t *s = (int16_t *)stream;
  for (int i = 0; i < len / 2; i++) {
    int16_t v = (int16_t)(((nproduced % 4000) + 1) * 7);
    s[i] = v;
    if (nproduced < 200000) produced[nproduced] = v;
    nproduced++;
  }
}
static const char *err(void) { const char *e = p_SDL_GetError ? p_SDL_GetError() : NULL; return e ? e : ""; }

int main(void) {
  QT_BEGIN("sdl2-compat");
  void *h = dlopen("libSDL2-2.0.so.0", RTLD_NOW | RTLD_LOCAL);
  if (!h) { const char *de = dlerror(); printf("  info dlopen: %s\n", de ? de : "?"); }
  QT_CHECK(h != NULL, "dlopen libSDL2-2.0.so.0");
  if (!h) QT_END();
  int ok = 1;
  LOAD(SDL_GetVersion) LOAD(SDL_Init) LOAD(SDL_Quit) LOAD(SDL_GetError) LOAD(SDL_GetCurrentVideoDriver)
  LOAD(SDL_GetCurrentAudioDriver) LOAD(SDL_CreateRGBSurfaceWithFormat) LOAD(SDL_FreeSurface) LOAD(SDL_FillRect)
  LOAD(SDL_MapRGB) LOAD(SDL_CreateSoftwareRenderer) LOAD(SDL_SetRenderDrawColor) LOAD(SDL_RenderFillRect)
  LOAD(SDL_RenderPresent) LOAD(SDL_RenderReadPixels) LOAD(SDL_DestroyRenderer) LOAD(SDL_CreateWindow)
  LOAD(SDL_GetWindowSurface) LOAD(SDL_UpdateWindowSurface) LOAD(SDL_DestroyWindow) LOAD(SDL_GetWindowSize)
  LOAD(SDL_OpenAudioDevice) LOAD(SDL_PauseAudioDevice) LOAD(SDL_CloseAudioDevice) LOAD(SDL_Delay)
  LOAD(SDL_GetTicks)
  QT_CHECK(ok, "all SDL2 entry points resolve");
  if (!ok) QT_END();
  void *h3 = dlopen("libSDL3.so.0", RTLD_NOW | RTLD_NOLOAD);
  QT_CHECK(h3 != NULL, "sdl2-compat loaded libSDL3.so.0");
  if (h3) dlclose(h3);

  SDL_version v; p_SDL_GetVersion(&v);
  printf("  info SDL_GetVersion: %u.%u.%u\n", v.major, v.minor, v.patch);
  QT_CHECK(v.major == 2 && v.minor == 32 && v.patch == 60, "SDL2 API version 2.32.60");

  char dir[64] = "/tmp/qnxport-sdl2compat.XXXXXX";
  QT_CHECK(mkdtemp(dir) != NULL, "scratch directory");
  char raw[128]; snprintf(raw, sizeof raw, "%s/audio.raw", dir);
  setenv("SDL_VIDEODRIVER", "dummy", 1);
  setenv("SDL_AUDIODRIVER", "disk", 1);
  setenv("SDL_DISKAUDIOFILE", raw, 1);
  int rc = p_SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_EVENTS | SDL_INIT_TIMER);
  if (rc) printf("  info SDL_Init: %s\n", err());
  QT_CHECK(rc == 0, "SDL_Init(VIDEO|AUDIO|EVENTS|TIMER)");
  const char *vd = p_SDL_GetCurrentVideoDriver(), *ad = p_SDL_GetCurrentAudioDriver();
  printf("  info video driver %s, audio driver %s\n", vd ? vd : "(none)", ad ? ad : "(none)");
  QT_CHECK(vd && !strcmp(vd, "dummy"), "video driver is dummy (SDL_VIDEODRIVER honoured)");
  QT_CHECK(ad && !strcmp(ad, "disk"), "audio driver is disk (SDL_AUDIODRIVER honoured)");

  /* ---- surfaces and the software renderer ---- */
  SDL_Surface *s = p_SDL_CreateRGBSurfaceWithFormat(0, 64, 48, 32, SDL_PIXELFORMAT_ARGB8888);
  QT_CHECK(s && s->w == 64 && s->h == 48 && s->format->BytesPerPixel == 4, "64x48 ARGB8888 surface");
  if (s) {
    QT_CHECK(p_SDL_FillRect(s, NULL, p_SDL_MapRGB(s->format, 0, 0, 255)) == 0, "SDL_FillRect: blue background");
    SDL_Rect r = { 8, 8, 16, 16 };
    p_SDL_FillRect(s, &r, p_SDL_MapRGB(s->format, 255, 0, 0));
    SDL_Renderer *rd = p_SDL_CreateSoftwareRenderer(s);
    if (!rd) printf("  info SDL_CreateSoftwareRenderer: %s\n", err());
    QT_CHECK(rd != NULL, "SDL_CreateSoftwareRenderer");
    if (rd) {
      p_SDL_SetRenderDrawColor(rd, 0, 255, 0, 255);
      SDL_Rect g = { 40, 8, 16, 16 };
      QT_CHECK(p_SDL_RenderFillRect(rd, &g) == 0, "SDL_RenderFillRect: green square");
      static Uint32 px[64 * 48];
      rc = p_SDL_RenderReadPixels(rd, NULL, SDL_PIXELFORMAT_ARGB8888, px, 64 * 4);
      p_SDL_RenderPresent(rd);
      QT_CHECK(rc == 0, "SDL_RenderReadPixels");
      printf("  info pixels: (2,2)=%08x (16,16)=%08x (48,16)=%08x\n", (unsigned)px[2 * 64 + 2],
             (unsigned)px[16 * 64 + 16], (unsigned)px[16 * 64 + 48]);
      QT_CHECK((px[2 * 64 + 2] & 0xffffff) == 0x0000ff, "background pixel is blue");
      QT_CHECK((px[16 * 64 + 16] & 0xffffff) == 0xff0000, "SDL_FillRect square is red");
      QT_CHECK((px[16 * 64 + 48] & 0xffffff) == 0x00ff00, "renderer square is green");
      p_SDL_DestroyRenderer(rd);
    }
    p_SDL_FreeSurface(s);
  }

  /* ---- a window on the dummy driver ---- */
  SDL_Window *w = p_SDL_CreateWindow("qnx sdl2-compat", 0, 0, 160, 120, 0);
  if (!w) printf("  info SDL_CreateWindow: %s\n", err());
  QT_CHECK(w != NULL, "SDL_CreateWindow on the dummy driver");
  if (w) {
    int ww = 0, wh = 0; p_SDL_GetWindowSize(w, &ww, &wh);
    QT_CHECK(ww == 160 && wh == 120, "window is 160x120");
    SDL_Surface *ws = p_SDL_GetWindowSurface(w);
    QT_CHECK(ws && ws->w == 160 && ws->h == 120, "window surface 160x120");
    if (ws) {
      p_SDL_FillRect(ws, NULL, p_SDL_MapRGB(ws->format, 10, 20, 30));
      QT_CHECK(p_SDL_UpdateWindowSurface(w) == 0, "SDL_UpdateWindowSurface");
    }
    p_SDL_DestroyWindow(w);
  }

  /* ---- audio to the disk driver ---- */
  SDL_AudioSpec want, have;
  memset(&want, 0, sizeof want); memset(&have, 0, sizeof have);
  /* SDL3 never opens a physical device below its defaults (44100 Hz, stereo,
     S16 for the disk driver): an 8 kHz mono request is resampled on the way
     to the file. Ask for exactly the device format, so SDL3's "simple copy"
     path hands the callback's samples to the driver unchanged. */
  want.freq = 44100; want.format = AUDIO_S16SYS; want.channels = 2; want.samples = 1024; want.callback = audio_cb;
  SDL_AudioDeviceID dev = p_SDL_OpenAudioDevice(NULL, 0, &want, &have, 0);
  if (!dev) printf("  info SDL_OpenAudioDevice: %s\n", err());
  QT_CHECK(dev != 0, "SDL_OpenAudioDevice (callback, 44.1 kHz S16 stereo)");
  if (dev) {
    printf("  info obtained %d Hz, format 0x%x, %u channel(s), %u samples\n", have.freq, have.format,
           have.channels, have.samples);
    QT_CHECK(have.freq == 44100 && have.format == AUDIO_S16SYS && have.channels == 2, "obtained spec as requested");
    Uint32 t0 = p_SDL_GetTicks();
    p_SDL_PauseAudioDevice(dev, 0);
    p_SDL_Delay(700);
    p_SDL_CloseAudioDevice(dev);
    printf("  info callback produced %ld samples in %u ms\n", nproduced, (unsigned)(p_SDL_GetTicks() - t0));
    QT_CHECK(nproduced >= 2048, "the callback was pulled (>= 4 buffers)");
    FILE *f = fopen(raw, "rb");
    static int16_t got[200000]; long n = 0;
    if (f) { n = (long)fread(got, 2, 200000, f); fclose(f); }
    long lead = 0; while (lead < n && got[lead] == 0) lead++;
    while (n > lead && got[n - 1] == 0) n--;   /* the callback never produces 0 */
    long same = 0, cmp = n - lead < nproduced ? n - lead : nproduced;
    if (cmp > 200000) cmp = 200000;
    for (long i = 0; i < cmp; i++) if (got[lead + i] == produced[i]) same++;
    printf("  info audio.raw: %ld samples (%ld leading silence), %ld/%ld match the callback's\n", n, lead, same, cmp);
    QT_CHECK(cmp >= 2048, "the disk driver wrote the audio");
    QT_CHECK(cmp > 0 && same == cmp, "written samples are exactly the callback's");
  }
  p_SDL_Quit();
  char cmd[128]; snprintf(cmd, sizeof cmd, "rm -rf %s", dir); system(cmd);
  QT_END();
}
