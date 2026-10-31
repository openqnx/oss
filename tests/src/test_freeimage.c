/* FreeImage: build a 24-bit test image pixel by pixel, save it to memory as
 * PNG, BMP and TIFF and load each back losslessly (format sniffing too),
 * JPEG within a tolerance, then flip, greyscale and rescale with known
 * results. A C program: libfreeimage.so brings its own C++ runtime. */
#include "common.h"
#include <stdio.h>
#include <stdlib.h>
#include <FreeImage.h>

#define W 16
#define H 8
static void colour(int x, int y, RGBQUAD *q) {
  q->rgbRed = (BYTE)(x * 16); q->rgbGreen = (BYTE)(y * 32); q->rgbBlue = (BYTE)((x + y) * 8); q->rgbReserved = 0;
}
static int same(FIBITMAP *a, int tol) {
  if (!a || FreeImage_GetWidth(a) != W || FreeImage_GetHeight(a) != H) return 0;
  for (int y = 0; y < H; ++y)
    for (int x = 0; x < W; ++x) {
      RGBQUAD p, q;
      FreeImage_GetPixelColor(a, x, y, &p); colour(x, y, &q);
      if (abs(p.rgbRed - q.rgbRed) > tol || abs(p.rgbGreen - q.rgbGreen) > tol || abs(p.rgbBlue - q.rgbBlue) > tol) return 0;
    }
  return 1;
}
static FIBITMAP *round_trip(FIBITMAP *src, FREE_IMAGE_FORMAT fif, int flags, FREE_IMAGE_FORMAT *sniffed, DWORD *bytes) {
  FIMEMORY *m = FreeImage_OpenMemory(NULL, 0);
  FIBITMAP *out = NULL;
  if (FreeImage_SaveToMemory(fif, src, m, flags)) {
    BYTE *d; DWORD n;
    FreeImage_AcquireMemory(m, &d, &n);
    *bytes = n;
    FreeImage_SeekMemory(m, 0, SEEK_SET);
    *sniffed = FreeImage_GetFileTypeFromMemory(m, 0);
    FreeImage_SeekMemory(m, 0, SEEK_SET);
    out = FreeImage_LoadFromMemory(fif, m, 0);
  }
  FreeImage_CloseMemory(m);
  return out;
}

int main(void) {
  QT_BEGIN("freeimage");
  FreeImage_Initialise(FALSE);
  printf("  info FreeImage %s\n", FreeImage_GetVersion());
  FIBITMAP *img = FreeImage_Allocate(W, H, 24, 0, 0, 0);
  QT_CHECK(img && FreeImage_GetBPP(img) == 24, "allocate a 16x8 24-bit bitmap");
  for (int y = 0; y < H; ++y) for (int x = 0; x < W; ++x) { RGBQUAD q; colour(x, y, &q); FreeImage_SetPixelColor(img, x, y, &q); }
  QT_CHECK(same(img, 0), "pixels set and read back");

  struct { FREE_IMAGE_FORMAT f; const char *n; int flags, tol; } fmts[] = {
    {FIF_PNG, "PNG", 0, 0}, {FIF_BMP, "BMP", 0, 0}, {FIF_TIFF, "TIFF", 0, 0}, {FIF_JPEG, "JPEG", JPEG_QUALITYSUPERB | JPEG_SUBSAMPLING_444, 24}};
  for (int i = 0; i < 4; ++i) {
    FREE_IMAGE_FORMAT sn = FIF_UNKNOWN; DWORD n = 0;
    char what[96];
    snprintf(what, sizeof what, "%s plugin reads and writes", fmts[i].n);
    QT_CHECK(FreeImage_FIFSupportsWriting(fmts[i].f) && FreeImage_FIFSupportsReading(fmts[i].f), what);
    FIBITMAP *b = round_trip(img, fmts[i].f, fmts[i].flags, &sn, &n);
    printf("  info %s: %lu bytes\n", fmts[i].n, (unsigned long)n);
    snprintf(what, sizeof what, "%s: encoded stream is recognised as %s", fmts[i].n, fmts[i].n);
    QT_CHECK(sn == fmts[i].f, what);
    snprintf(what, sizeof what, "%s round trip %s", fmts[i].n, fmts[i].tol ? "(within JPEG tolerance)" : "is lossless");
    QT_CHECK(same(b, fmts[i].tol), what);
    if (b) FreeImage_Unload(b);
  }

  FIBITMAP *flip = FreeImage_Clone(img);
  FreeImage_FlipHorizontal(flip);
  RGBQUAD p; FreeImage_GetPixelColor(flip, W - 1, 3, &p);
  QT_CHECK(p.rgbRed == 0 && p.rgbGreen == 96 && p.rgbBlue == 24, "FlipHorizontal moves column 0 to column 15");
  FreeImage_Unload(flip);

  FIBITMAP *red = FreeImage_Allocate(4, 4, 24, 0, 0, 0);
  RGBQUAD r = {0, 0, 255, 0};   /* BGR order in RGBQUAD: blue, green, red */
  for (int y = 0; y < 4; ++y) for (int x = 0; x < 4; ++x) FreeImage_SetPixelColor(red, x, y, &r);
  FIBITMAP *g = FreeImage_ConvertToGreyscale(red);
  BYTE gv = 0; FreeImage_GetPixelIndex(g, 1, 1, &gv);
  printf("  info grey of pure red = %u\n", gv);
  QT_CHECK(g && FreeImage_GetBPP(g) == 8 && gv >= 53 && gv <= 77, "greyscale of pure red uses luma weights (~54 Rec.709 / ~76 Rec.601)");
  FIBITMAP *big = FreeImage_Rescale(red, 32, 16, FILTER_BILINEAR);
  RGBQUAD c; FreeImage_GetPixelColor(big, 20, 10, &c);
  QT_CHECK(big && FreeImage_GetWidth(big) == 32 && FreeImage_GetHeight(big) == 16 && c.rgbRed >= 250 && c.rgbGreen <= 5,
           "rescale a uniform image to 32x16 keeps its colour");
  FreeImage_Unload(big); FreeImage_Unload(g); FreeImage_Unload(red);
  FreeImage_Unload(img);
  FreeImage_DeInitialise();
  QT_END();
}
