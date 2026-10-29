/* SDL2: initialise the subsystems that make sense headless, and exercise the
 * timer, event queue, RWops, endian and CPU-feature layers. Video is probed
 * but a target with no compositor running is a legitimate outcome. */
#include "common.h"
#include <stdlib.h>
#include <SDL2/SDL.h>
static Uint32 tick(Uint32 interval, void *param) {
  *(int *)param += 1;
  return 0;                       /* one-shot */
}
int main(int argc, char **argv) {
  (void)argc; (void)argv;
  QT_BEGIN("sdl2");
  SDL_version v;
  SDL_GetVersion(&v);
  printf("  info SDL %d.%d.%d\n", v.major, v.minor, v.patch);
  QT_CHECK(SDL_Init(SDL_INIT_TIMER | SDL_INIT_EVENTS) == 0, "SDL_Init(TIMER|EVENTS)");
  if (SDL_WasInit(SDL_INIT_TIMER) == 0) { printf("  info %s\n", SDL_GetError()); }

  /* --- timers --- */
  int fired = 0;
  SDL_TimerID id = SDL_AddTimer(30, tick, &fired);
  QT_CHECK(id != 0, "SDL_AddTimer");
  Uint64 start = SDL_GetPerformanceCounter();
  SDL_Delay(120);
  QT_CHECK(fired == 1, "timer callback fired");
  QT_CHECK(SDL_GetPerformanceCounter() > start, "performance counter advances");
  QT_CHECK(SDL_GetTicks64() > 0, "SDL_GetTicks64");

  /* --- event queue --- */
  SDL_Event ev;
  SDL_memset(&ev, 0, sizeof ev);
  ev.type = SDL_USEREVENT;
  ev.user.code = 8;
  QT_CHECK(SDL_PushEvent(&ev) == 1, "SDL_PushEvent");
  SDL_Event got;
  QT_CHECK(SDL_PollEvent(&got) == 1, "SDL_PollEvent");
  QT_CHECK(got.type == SDL_USEREVENT && got.user.code == 8, "the event round-tripped");

  /* --- RWops over memory --- */
  char buf[32] = {0};
  SDL_RWops *rw = SDL_RWFromMem(buf, sizeof buf);
  QT_CHECK(rw != NULL, "SDL_RWFromMem");
  QT_CHECK(SDL_RWwrite(rw, "qnx", 1, 3) == 3, "SDL_RWwrite");
  QT_CHECK(SDL_RWseek(rw, 0, RW_SEEK_SET) == 0, "SDL_RWseek");
  char back[4] = {0};
  QT_CHECK(SDL_RWread(rw, back, 1, 3) == 3 && strcmp(back, "qnx") == 0, "SDL_RWread");
  SDL_RWclose(rw);

  /* --- byte order and CPU features --- */
  QT_CHECK(SDL_Swap32(0x12345678u) == 0x78563412u, "SDL_Swap32");
  QT_CHECK(SDL_GetCPUCount() > 0, "SDL_GetCPUCount");
  printf("  info %d cpu(s), %d MiB RAM, %d-byte cache line\n",
         SDL_GetCPUCount(), SDL_GetSystemRAM(), SDL_GetCPUCacheLineSize());

  /* --- drivers: enumerate rather than require, since a headless target has
   *     no compositor to connect to --- */
  int nvid = SDL_GetNumVideoDrivers();
  printf("  info %d video driver(s):", nvid);
  for (int i = 0; i < nvid; ++i) printf(" %s", SDL_GetVideoDriver(i));
  printf("\n");
  QT_CHECK(nvid > 0, "at least one video driver is compiled in");
  int naud = SDL_GetNumAudioDrivers();
  printf("  info %d audio driver(s):", naud);
  for (int i = 0; i < naud; ++i) printf(" %s", SDL_GetAudioDriver(i));
  printf("\n");
  QT_CHECK(naud > 0, "at least one audio driver is compiled in");
  int has_alsa = 0;
  for (int i = 0; i < naud; ++i)
    if (strcmp(SDL_GetAudioDriver(i), "alsa") == 0) has_alsa = 1;
  QT_CHECK(has_alsa, "the ALSA backend is built (QNX 8 ships alsa-lib, not QSA)");

  if (SDL_VideoInit(NULL) == 0) {
    printf("  info video initialised with \"%s\"\n", SDL_GetCurrentVideoDriver());
    SDL_VideoQuit();
  } else {
    printf("  info no display available: %s\n", SDL_GetError());
  }
  SDL_RemoveTimer(id);
  SDL_Quit();
  QT_END();
}
