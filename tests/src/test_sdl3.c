/* SDL3 without a display: events (register + push + poll a user event),
 * properties and hints, CRC-32 vector, surfaces (fill, read pixel, scaled
 * blit), BMP save/load through a dynamic-memory SDL_IOStream, audio format
 * conversion with SDL_AudioStream, a thread + atomic counter, and the dummy
 * video driver: a window with the software renderer, clear to a colour and
 * read the pixels back. */
#include "common.h"
#include <SDL3/SDL.h>
static SDL_AtomicInt counter;
static int SDLCALL worker(void *p) { for (int i = 0; i < 1000; i++) SDL_AddAtomicInt(&counter, 1); return (int)(intptr_t)p; }
int main(void) {
  QT_BEGIN("sdl3");
  int v = SDL_GetVersion();
  printf("  info SDL %d.%d.%d (%s)\n", SDL_VERSIONNUM_MAJOR(v), SDL_VERSIONNUM_MINOR(v), SDL_VERSIONNUM_MICRO(v), SDL_GetRevision());
  QT_CHECK(SDL_VERSIONNUM_MAJOR(v) == 3, "SDL major version 3");
  QT_CHECK(SDL_SetHint(SDL_HINT_VIDEO_DRIVER, "dummy"), "set video driver hint");
  QT_CHECK(SDL_Init(SDL_INIT_EVENTS), "SDL_Init(EVENTS)");

  Uint32 ev = SDL_RegisterEvents(1);
  QT_CHECK(ev != 0, "SDL_RegisterEvents");
  SDL_Event e; SDL_zero(e); e.type = ev; e.user.code = 4242; e.user.data1 = (void *)&counter;
  QT_CHECK(SDL_PushEvent(&e), "SDL_PushEvent");
  int found = 0; SDL_Event g;
  while (SDL_PollEvent(&g)) if (g.type == ev && g.user.code == 4242 && g.user.data1 == (void *)&counter) found = 1;
  QT_CHECK(found, "user event polled back with its payload");

  SDL_PropertiesID pr = SDL_CreateProperties();
  SDL_SetNumberProperty(pr, "qnx.n", 1234567890123LL); SDL_SetStringProperty(pr, "qnx.s", "hello");
  SDL_SetFloatProperty(pr, "qnx.f", 2.5f); SDL_SetBooleanProperty(pr, "qnx.b", true);
  const char *ps = SDL_GetStringProperty(pr, "qnx.s", NULL);
  QT_CHECK(SDL_GetNumberProperty(pr, "qnx.n", 0) == 1234567890123LL && ps && strcmp(ps, "hello") == 0 &&
           SDL_GetFloatProperty(pr, "qnx.f", 0) == 2.5f && SDL_GetBooleanProperty(pr, "qnx.b", false) &&
           SDL_GetNumberProperty(pr, "qnx.none", -1) == -1, "properties set/get (number, string, float, bool, default)");
  SDL_DestroyProperties(pr);
  SDL_SetHint("QNXPORT_TEST_HINT", "on");
  const char *h = SDL_GetHint("QNXPORT_TEST_HINT");
  QT_CHECK(h && strcmp(h, "on") == 0 && SDL_GetHintBoolean("QNXPORT_TEST_HINT", false), "hint round trip");

  Uint32 crc = SDL_crc32(0, "123456789", 9);
  printf("  info SDL_crc32(\"123456789\") = %08x\n", (unsigned)crc);
  QT_CHECK(crc == 0xCBF43926u, "SDL_crc32 check value");

  SDL_Surface *s = SDL_CreateSurface(32, 16, SDL_PIXELFORMAT_RGBA8888);
  QT_CHECK(s != NULL, "SDL_CreateSurface 32x16 RGBA8888");
  if (s) {
    const SDL_PixelFormatDetails *fd = SDL_GetPixelFormatDetails(s->format);
    SDL_FillSurfaceRect(s, NULL, SDL_MapRGBA(fd, NULL, 10, 20, 30, 255));
    SDL_Rect rc = { 8, 4, 8, 8 };
    SDL_FillSurfaceRect(s, &rc, SDL_MapRGBA(fd, NULL, 200, 100, 50, 255));
    Uint8 r, gg, b, a;
    SDL_ReadSurfacePixel(s, 10, 6, &r, &gg, &b, &a);
    int in = r == 200 && gg == 100 && b == 50 && a == 255;
    SDL_ReadSurfacePixel(s, 0, 0, &r, &gg, &b, &a);
    QT_CHECK(in && r == 10 && gg == 20 && b == 30, "fill rect + read pixels inside and outside");
    SDL_Surface *big = SDL_CreateSurface(64, 32, SDL_PIXELFORMAT_XRGB8888);
    QT_CHECK(big && SDL_BlitSurfaceScaled(s, NULL, big, NULL, SDL_SCALEMODE_NEAREST), "scaled blit 2x, format conversion");
    if (big) {
      SDL_ReadSurfacePixel(big, 20, 12, &r, &gg, &b, &a);
      QT_CHECK(r == 200 && gg == 100 && b == 50, "scaled pixel lands at 2x position");
      SDL_DestroySurface(big);
    }
    SDL_IOStream *io = SDL_IOFromDynamicMem();
    QT_CHECK(io && SDL_SaveBMP_IO(s, io, false), "SaveBMP into dynamic memory");
    Sint64 sz = io ? SDL_GetIOSize(io) : -1;
    printf("  info BMP %lld bytes\n", (long long)sz);
    if (io) {
      SDL_SeekIO(io, 0, SDL_IO_SEEK_SET);
      Uint8 hdr[2] = { 0 }; SDL_ReadIO(io, hdr, 2);
      SDL_SeekIO(io, 0, SDL_IO_SEEK_SET);
      SDL_Surface *l = SDL_LoadBMP_IO(io, true);
      QT_CHECK(hdr[0] == 'B' && hdr[1] == 'M' && l && l->w == 32 && l->h == 16, "LoadBMP back (BM magic, 32x16)");
      if (l) {
        SDL_ReadSurfacePixel(l, 12, 10, &r, &gg, &b, &a);
        QT_CHECK(r == 200 && gg == 100 && b == 50, "BMP round-trip pixel");
        SDL_DestroySurface(l);
      }
    }
    SDL_DestroySurface(s);
  }

  SDL_AudioSpec in = { SDL_AUDIO_S16, 1, 8000 }, out = { SDL_AUDIO_F32, 2, 8000 };
  SDL_AudioStream *as = SDL_CreateAudioStream(&in, &out);
  QT_CHECK(as != NULL, "SDL_CreateAudioStream S16 mono -> F32 stereo");
  if (as) {
    Sint16 smp[4] = { 0, 16384, -16384, 32767 };
    float o[8] = { 0 };
    SDL_PutAudioStreamData(as, smp, sizeof smp); SDL_FlushAudioStream(as);
    int n = SDL_GetAudioStreamData(as, o, sizeof o);
    printf("  info converted %d bytes: %g %g | %g %g | %g\n", n, o[0], o[1], o[2], o[3], o[4]);
    QT_CHECK(n == (int)sizeof o && o[2] == 0.5f && o[3] == 0.5f && o[4] == -0.5f && o[6] > 0.99f, "sample values converted and duplicated");
    SDL_DestroyAudioStream(as);
  }

  SDL_SetAtomicInt(&counter, 0);
  SDL_Thread *t1 = SDL_CreateThread(worker, "w1", (void *)7), *t2 = SDL_CreateThread(worker, "w2", (void *)9);
  int r1 = 0, r2 = 0; SDL_WaitThread(t1, &r1); SDL_WaitThread(t2, &r2);
  QT_CHECK(t1 && t2 && r1 == 7 && r2 == 9 && SDL_GetAtomicInt(&counter) == 2000, "two threads, return codes, atomic sum 2000");
  printf("  info %d logical CPU core(s)\n", SDL_GetNumLogicalCPUCores());

  int vid = SDL_InitSubSystem(SDL_INIT_VIDEO);
  const char *vd = SDL_GetCurrentVideoDriver();
  printf("  info video driver: %s%s%s\n", vd ? vd : "(none)", vid ? "" : " -- ", vid ? "" : SDL_GetError());
  QT_CHECK(vid && vd && strcmp(vd, "dummy") == 0, "video subsystem on the dummy driver");
  if (vid) {
    SDL_Window *w = NULL; SDL_Renderer *rd = NULL;
    QT_CHECK(SDL_CreateWindowAndRenderer("qnx", 40, 30, 0, &w, &rd), "window + renderer");
    if (rd) {
      const char *rn = SDL_GetRendererName(rd);
      printf("  info renderer %s\n", rn ? rn : "(null)");
      SDL_SetRenderDrawColor(rd, 0, 128, 255, 255); SDL_RenderClear(rd);
      SDL_SetRenderDrawColor(rd, 255, 255, 0, 255); SDL_FRect fr = { 10, 10, 5, 5 }; SDL_RenderFillRect(rd, &fr);
      SDL_Surface *p = SDL_RenderReadPixels(rd, NULL);
      Uint8 r, gg, b, a, r2 = 0, g2 = 0, b2 = 0;
      int ok = p && SDL_ReadSurfacePixel(p, 2, 2, &r, &gg, &b, &a) && SDL_ReadSurfacePixel(p, 12, 12, &r2, &g2, &b2, &a);
      QT_CHECK(ok && r == 0 && gg == 128 && b == 255 && r2 == 255 && g2 == 255 && b2 == 0, "rendered pixels read back");
      if (p) SDL_DestroySurface(p);
      SDL_DestroyRenderer(rd);
    }
    if (w) SDL_DestroyWindow(w);
  }
  SDL_Quit();
  QT_END();
}
