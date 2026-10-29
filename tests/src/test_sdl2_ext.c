/* SDL2_image, SDL2_ttf and SDL2_net: the three extension libraries, exercised
 * without a display. Image decoding, font rasterisation and socket setup all
 * work headless. */
#include "common.h"
#include <stdlib.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_net.h>
/* A 1x1 red PNG, so the test carries its own input. */
static const unsigned char kPng[] = {
  0x89,0x50,0x4E,0x47,0x0D,0x0A,0x1A,0x0A,0x00,0x00,0x00,0x0D,0x49,0x48,0x44,0x52,
  0x00,0x00,0x00,0x01,0x00,0x00,0x00,0x01,0x08,0x02,0x00,0x00,0x00,0x90,0x77,0x53,
  0xDE,0x00,0x00,0x00,0x0C,0x49,0x44,0x41,0x54,0x08,0xD7,0x63,0xF8,0xCF,0xC0,0x00,
  0x00,0x03,0x01,0x01,0x00,0x18,0xDD,0x8D,0xB0,0x00,0x00,0x00,0x00,0x49,0x45,0x4E,
  0x44,0xAE,0x42,0x60,0x82
};
static const char *fonts[] = {
  "/usr/local/share/fonts/DejaVuSans.ttf",
  "/usr/share/fonts/DejaVuSans.ttf",
  "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf",
  NULL
};
int main(int argc, char **argv) {
  (void)argc; (void)argv;
  QT_BEGIN("sdl2 extensions");
  QT_CHECK(SDL_Init(SDL_INIT_TIMER | SDL_INIT_EVENTS) == 0, "SDL_Init");

  /* --- SDL2_image --- */
  int want = IMG_INIT_PNG | IMG_INIT_JPG;
  int got = IMG_Init(want);
  printf("  info IMG_Init -> PNG:%d JPG:%d WEBP:%d AVIF:%d\n",
         !!(got & IMG_INIT_PNG), !!(got & IMG_INIT_JPG),
         !!(got & IMG_INIT_WEBP), !!(got & IMG_INIT_AVIF));
  QT_CHECK((got & IMG_INIT_PNG) != 0, "PNG decoder registered");
  SDL_RWops *rw = SDL_RWFromConstMem(kPng, sizeof kPng);
  QT_CHECK(rw != NULL, "SDL_RWFromConstMem");
  QT_CHECK(IMG_isPNG(rw), "IMG_isPNG recognises the data");
  SDL_Surface *img = IMG_Load_RW(rw, 1);
  QT_CHECK(img != NULL, "IMG_Load_RW decoded the PNG");
  if (img) {
    QT_CHECK(img->w == 1 && img->h == 1, "decoded size is 1x1");
    SDL_FreeSurface(img);
  } else {
    printf("  info %s\n", IMG_GetError());
  }
  IMG_Quit();

  /* --- SDL2_ttf --- */
  QT_CHECK(TTF_Init() == 0, "TTF_Init");
  SDL_version tv;
  SDL_TTF_VERSION(&tv);
  printf("  info SDL2_ttf %d.%d.%d\n", tv.major, tv.minor, tv.patch);
  TTF_Font *font = NULL;
  for (int i = 0; fonts[i] && !font; ++i) font = TTF_OpenFont(fonts[i], 24);
  if (font) {
    int w = 0, h = 0;
    QT_CHECK(TTF_SizeUTF8(font, "QNX", &w, &h) == 0 && w > 0 && h > 0,
             "TTF_SizeUTF8 measured the string");
    SDL_Color fg = { 255, 255, 255, 255 };
    SDL_Surface *text = TTF_RenderUTF8_Blended(font, "QNX", fg);
    QT_CHECK(text != NULL && text->w > 0, "TTF_RenderUTF8_Blended rasterised");
    if (text) SDL_FreeSurface(text);
    TTF_CloseFont(font);
  } else {
    printf("  info no TTF on this target; checking the error path only\n");
    QT_CHECK(TTF_OpenFont("/nonexistent.ttf", 12) == NULL, "missing font reports NULL");
  }
  TTF_Quit();

  /* --- SDL2_net --- */
  QT_CHECK(SDLNet_Init() == 0, "SDLNet_Init");
  IPaddress addr;
  QT_CHECK(SDLNet_ResolveHost(&addr, NULL, 0) == 0, "SDLNet_ResolveHost (any address)");
  /* SDL_net decides server vs client from the resolved host: NULL means
     "listen on this port", a real address means "connect to it". */
  QT_CHECK(SDLNet_ResolveHost(&addr, NULL, 9999) == 0, "resolve for listening");
  TCPsocket srv = SDLNet_TCP_Open(&addr);
  QT_CHECK(srv != NULL, "SDLNet_TCP_Open listening socket");
  if (srv) {
    IPaddress cli;
    SDLNet_ResolveHost(&cli, "127.0.0.1", 9999);
    TCPsocket c = SDLNet_TCP_Open(&cli);
    QT_CHECK(c != NULL, "client connected over loopback");
    if (c) {
      /* SDLNet_TCP_Accept never blocks: it returns NULL when no connection is
         pending yet, so poll for a moment rather than assuming the handshake
         has already completed. */
      TCPsocket accepted = NULL;
      for (int i = 0; i < 100 && !accepted; ++i) {
        accepted = SDLNet_TCP_Accept(srv);
        if (!accepted) SDL_Delay(10);
      }
      QT_CHECK(accepted != NULL, "server accepted the connection");
      if (accepted) {
        QT_CHECK(SDLNet_TCP_Send(c, "qnx", 3) == 3, "SDLNet_TCP_Send");
        char buf[8] = {0};
        QT_CHECK(SDLNet_TCP_Recv(accepted, buf, 3) == 3 && strcmp(buf, "qnx") == 0,
                 "SDLNet_TCP_Recv got the bytes");
        SDLNet_TCP_Close(accepted);
      }
      SDLNet_TCP_Close(c);
    }
    SDLNet_TCP_Close(srv);
  }
  SDLNet_Quit();
  SDL_Quit();
  QT_END();
}
