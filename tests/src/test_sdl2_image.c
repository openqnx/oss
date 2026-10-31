/* SDL2_image: decode a known 1x1 PNG from memory (exact pixel), save a
 * generated 64x48 RGBA pattern as PNG to /tmp and load it back pixel-exact,
 * save/load JPEG (lossy: within tolerance), format sniffing (IMG_isPNG /
 * IMG_isJPG / IMG_isBMP), and a TGA via the typed loader. No display. */
#include "common.h"
#include <stdlib.h>
#include <unistd.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
static const unsigned char kPng[] = {   /* 1x1, RGB (255,0,0) */
  0x89,0x50,0x4E,0x47,0x0D,0x0A,0x1A,0x0A,0x00,0x00,0x00,0x0D,0x49,0x48,0x44,0x52,
  0x00,0x00,0x00,0x01,0x00,0x00,0x00,0x01,0x08,0x02,0x00,0x00,0x00,0x90,0x77,0x53,
  0xDE,0x00,0x00,0x00,0x0C,0x49,0x44,0x41,0x54,0x08,0xD7,0x63,0xF8,0xCF,0xC0,0x00,
  0x00,0x03,0x01,0x01,0x00,0x18,0xDD,0x8D,0xB0,0x00,0x00,0x00,0x00,0x49,0x45,0x4E,
  0x44,0xAE,0x42,0x60,0x82 };
/* uncompressed 2x1 true-colour TGA, bottom-up: blue, green (BGR order) */
static const unsigned char kTga[] = { 0,0,2, 0,0,0,0,0, 0,0,0,0, 2,0,1,0, 24,0, 255,0,0, 0,255,0 };
static Uint32 px(SDL_Surface *s, int x, int y) { return ((Uint32 *)((Uint8 *)s->pixels + y * s->pitch))[x]; }
static void pattern(int x, int y, Uint8 *r, Uint8 *g, Uint8 *b, Uint8 *a) {
  *r = (Uint8)(x * 4); *g = (Uint8)(y * 5); *b = (Uint8)((x ^ y) * 7); *a = (Uint8)(255 - x - y);
}
int main(void) {
  QT_BEGIN("sdl2-image");
  const SDL_version *v = IMG_Linked_Version();
  printf("  info SDL2_image %d.%d.%d\n", v->major, v->minor, v->patch);
  int want = IMG_INIT_PNG | IMG_INIT_JPG | IMG_INIT_TIF | IMG_INIT_WEBP | IMG_INIT_AVIF | IMG_INIT_JXL;
  int got = IMG_Init(want);
  printf("  info IMG_Init: png=%d jpg=%d tif=%d webp=%d avif=%d jxl=%d\n", !!(got & IMG_INIT_PNG), !!(got & IMG_INIT_JPG),
         !!(got & IMG_INIT_TIF), !!(got & IMG_INIT_WEBP), !!(got & IMG_INIT_AVIF), !!(got & IMG_INIT_JXL));
  QT_CHECK((got & (IMG_INIT_PNG | IMG_INIT_JPG)) == (IMG_INIT_PNG | IMG_INIT_JPG), "PNG and JPEG codecs available");

  SDL_RWops *rw = SDL_RWFromConstMem(kPng, sizeof kPng);
  QT_CHECK(IMG_isPNG(rw) && !IMG_isJPG(rw) && !IMG_isBMP(rw), "format sniffing: PNG");
  SDL_Surface *s = IMG_Load_RW(rw, 1);
  QT_CHECK(s && s->w == 1 && s->h == 1, "IMG_Load_RW decodes the 1x1 PNG");
  if (s) {
    SDL_Surface *c = SDL_ConvertSurfaceFormat(s, SDL_PIXELFORMAT_RGBA32, 0);
    Uint8 r, g, b, a; SDL_GetRGBA(px(c, 0, 0), c->format, &r, &g, &b, &a);
    QT_CHECK(r == 255 && g == 0 && b == 0 && a == 255, "decoded pixel is pure red");
    SDL_FreeSurface(c); SDL_FreeSurface(s);
  }

  SDL_Surface *src = SDL_CreateRGBSurfaceWithFormat(0, 64, 48, 32, SDL_PIXELFORMAT_RGBA32);
  QT_CHECK(src != NULL, "create 64x48 RGBA surface");
  if (!src) QT_END();
  for (int y = 0; y < 48; y++) for (int x = 0; x < 64; x++) {
    Uint8 r, g, b, a; pattern(x, y, &r, &g, &b, &a);
    ((Uint32 *)((Uint8 *)src->pixels + y * src->pitch))[x] = SDL_MapRGBA(src->format, r, g, b, a);
  }
  char png[64], jpg[64]; int pid = (int)getpid();
  snprintf(png, sizeof png, "/tmp/qnxport-sdlimg-%d.png", pid); snprintf(jpg, sizeof jpg, "/tmp/qnxport-sdlimg-%d.jpg", pid);
  QT_CHECK(IMG_SavePNG(src, png) == 0, "IMG_SavePNG to /tmp");
  s = IMG_Load(png);
  QT_CHECK(s && s->w == 64 && s->h == 48, "IMG_Load reads it back (64x48)");
  if (s) {
    SDL_Surface *c = SDL_ConvertSurfaceFormat(s, SDL_PIXELFORMAT_RGBA32, 0);
    int bad = 0;
    for (int y = 0; y < 48; y++) for (int x = 0; x < 64; x++) {
      Uint8 r, g, b, a, R, G, B, A; pattern(x, y, &R, &G, &B, &A);
      SDL_GetRGBA(px(c, x, y), c->format, &r, &g, &b, &a);
      if (r != R || g != G || b != B || a != A) bad++;
    }
    printf("  info PNG mismatched pixels: %d\n", bad);
    QT_CHECK(bad == 0, "PNG round trip is lossless (RGBA)");
    SDL_FreeSurface(c); SDL_FreeSurface(s);
  }
  /* JPEG: opaque, smooth gradient */
  SDL_Surface *op = SDL_CreateRGBSurfaceWithFormat(0, 64, 48, 32, SDL_PIXELFORMAT_RGBA32);
  for (int y = 0; y < 48; y++) for (int x = 0; x < 64; x++)
    ((Uint32 *)((Uint8 *)op->pixels + y * op->pitch))[x] = SDL_MapRGBA(op->format, x * 4, y * 5, 128, 255);
  QT_CHECK(IMG_SaveJPG(op, jpg, 95) == 0, "IMG_SaveJPG quality 95");
  rw = SDL_RWFromFile(jpg, "rb");
  QT_CHECK(rw && IMG_isJPG(rw), "format sniffing: JPEG");
  s = rw ? IMG_LoadTyped_RW(rw, 1, "JPG") : NULL;
  if (s) {
    SDL_Surface *c = SDL_ConvertSurfaceFormat(s, SDL_PIXELFORMAT_RGBA32, 0);
    int maxd = 0;
    for (int y = 0; y < 48; y++) for (int x = 0; x < 64; x++) {
      Uint8 r, g, b, a; SDL_GetRGBA(px(c, x, y), c->format, &r, &g, &b, &a);
      int d = abs(r - x * 4); if (abs(g - y * 5) > d) d = abs(g - y * 5); if (abs(b - 128) > d) d = abs(b - 128);
      if (d > maxd) maxd = d;
    }
    printf("  info JPEG max channel error %d\n", maxd);
    QT_CHECK(s->w == 64 && s->h == 48 && maxd <= 20, "JPEG round trip within tolerance");
    SDL_FreeSurface(c); SDL_FreeSurface(s);
  } else QT_CHECK(0, "JPEG round trip within tolerance");

  s = IMG_LoadTGA_RW(SDL_RWFromConstMem(kTga, sizeof kTga));
  QT_CHECK(s && s->w == 2 && s->h == 1, "TGA decoded (2x1)");
  if (s) {
    SDL_Surface *c = SDL_ConvertSurfaceFormat(s, SDL_PIXELFORMAT_RGBA32, 0);
    Uint8 r, g, b, a, r2, g2, b2, a2;
    SDL_GetRGBA(px(c, 0, 0), c->format, &r, &g, &b, &a); SDL_GetRGBA(px(c, 1, 0), c->format, &r2, &g2, &b2, &a2);
    QT_CHECK(b == 255 && r == 0 && g == 0 && g2 == 255 && r2 == 0 && b2 == 0, "TGA pixels: blue, green");
    SDL_FreeSurface(c); SDL_FreeSurface(s);
  }
  QT_CHECK(IMG_Load("/tmp/qnxport-sdlimg-none.png") == NULL && IMG_GetError()[0], "missing file -> NULL with an error");
  SDL_FreeSurface(src); SDL_FreeSurface(op);
  unlink(png); unlink(jpg);
  IMG_Quit();
  QT_END();
}
