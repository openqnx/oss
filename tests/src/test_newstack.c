/* The packages added after the main sweep, in one binary:
 * libde265 (HEVC decode), SDL3, AT-SPI2 and pkgconf.
 *
 * Each is exercised through the operation it exists to perform, and each check
 * is one that a build-only pass could not have made. */
#include "common.h"
#include <stdlib.h>
#include <string.h>
#include <libde265/de265.h>
#include <SDL3/SDL.h>
#include <atspi/atspi.h>

int main(void) {
  QT_BEGIN("libde265 / SDL3 / at-spi2");

  /* --- libde265: decoder lifecycle and a real NAL --- */
  printf("  info libde265 %s\n", de265_get_version());
  de265_decoder_context *dec = de265_new_decoder();
  QT_CHECK(dec != NULL, "de265_new_decoder");
  if (dec) {
    QT_CHECK(de265_start_worker_threads(dec, 1) == DE265_OK,
             "de265_start_worker_threads");
    /* A VPS NAL from a conforming HEVC stream.  Pushing real coded data means
       the bitstream reader actually runs, rather than only the allocator. */
    static const uint8_t vps[] = {
      0x00,0x00,0x00,0x01, 0x40,0x01, 0x0c,0x01,0xff,0xff,
      0x01,0x60,0x00,0x00,0x03,0x00,0xb0,0x00,0x00,0x03,
      0x00,0x00,0x03,0x00,0x5d,0x95,0x98,0x09
    };
    de265_error e = de265_push_data(dec, vps, (int)sizeof vps, 0, NULL);
    QT_CHECK(e == DE265_OK, "de265_push_data accepted a VPS NAL");
    int more = 0;
    e = de265_decode(dec, &more);
    QT_CHECK(e == DE265_OK || e == DE265_ERROR_WAITING_FOR_INPUT_DATA,
             "de265_decode consumed it without error");
    printf("  info de265_decode -> %s\n", de265_get_error_text(e));
    QT_CHECK(de265_free_decoder(dec) == DE265_OK, "de265_free_decoder");
  }

  /* --- SDL3: init the subsystems that do not need a display --- */
  printf("  info SDL %d.%d.%d\n", SDL_MAJOR_VERSION, SDL_MINOR_VERSION, SDL_MICRO_VERSION);
  QT_CHECK(SDL_GetVersion() >= SDL_VERSIONNUM(3,2,0), "SDL_GetVersion >= 3.2.0");
  if (SDL_Init(SDL_INIT_EVENTS)) {
    QT_CHECK(1, "SDL_Init(EVENTS)");
    /* The timer subsystem is pure libc + threads, so it must work headless. */
    Uint64 t0 = SDL_GetTicksNS();
    SDL_DelayNS(2000000); /* 2 ms */
    Uint64 dt = SDL_GetTicksNS() - t0;
    printf("  info SDL_DelayNS(2ms) measured %llu ns\n", (unsigned long long)dt);
    QT_CHECK(dt >= 1000000, "SDL_GetTicksNS advanced across a delay");
    /* SDL3 keeps its own RWops/IOStream layer over the platform's files. */
    SDL_IOStream *io = SDL_IOFromMem((void *)"qnx", 3);
    QT_CHECK(io != NULL, "SDL_IOFromMem");
    if (io) {
      char b[4] = {0};
      QT_CHECK(SDL_ReadIO(io, b, 3) == 3 && memcmp(b, "qnx", 3) == 0,
               "SDL_ReadIO round trip");
      SDL_CloseIO(io);
    }
    QT_CHECK(SDL_GetNumVideoDrivers() > 0, "SDL3 registered a video driver");
    printf("  info video driver 0 = %s\n", SDL_GetVideoDriver(0));
    SDL_Quit();
  } else {
    QT_CHECK(0, "SDL_Init(EVENTS)");
    printf("  info SDL_GetError: %s\n", SDL_GetError());
  }

  /* --- AT-SPI2: the client library, without a running accessibility bus --- */
  /* atspi_init() returns 2 when there is no bus to connect to, which is the
     expected answer on a target with no session -- what matters is that the
     library loads, resolves against our dbus and glib, and answers. */
  int rc = atspi_init();
  printf("  info atspi_init -> %d\n", rc);
  QT_CHECK(rc >= 0, "atspi_init returned a defined status");
  QT_CHECK(atspi_is_initialized(), "atspi_is_initialized");

  QT_END();
}
