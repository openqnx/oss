/* giflib: write a small GIF to a file and read it back. */
#include "common.h"
#include <stdlib.h>
#include <unistd.h>
#include <gif_lib.h>
#define W 16
#define H 8
int main(void) {
  QT_BEGIN("giflib");
  char path[] = "/tmp/qnxport-gif-XXXXXX";
  int fd = mkstemp(path);
  QT_CHECK(fd >= 0, "create temp file");
  if (fd >= 0) close(fd);
  int err = 0;
  GifFileType *g = EGifOpenFileName(path, false, &err);
  QT_CHECK(g != NULL, "EGifOpenFileName");
  ColorMapObject *cmap = GifMakeMapObject(4, NULL);
  QT_CHECK(cmap != NULL, "GifMakeMapObject");
  for (int i = 0; i < 4; ++i) {
    cmap->Colors[i].Red = (GifByteType)(i * 60);
    cmap->Colors[i].Green = (GifByteType)(255 - i * 60);
    cmap->Colors[i].Blue = 0x20;
  }
  QT_CHECK(EGifPutScreenDesc(g, W, H, 2, 0, cmap) == GIF_OK, "EGifPutScreenDesc");
  QT_CHECK(EGifPutImageDesc(g, 0, 0, W, H, false, NULL) == GIF_OK, "EGifPutImageDesc");
  GifByteType line[W];
  for (int y = 0; y < H; ++y) {
    for (int x = 0; x < W; ++x) line[x] = (GifByteType)((x + y) % 4);
    EGifPutLine(g, line, W);
  }
  QT_CHECK(EGifCloseFile(g, &err) == GIF_OK, "EGifCloseFile");
  GifFreeMapObject(cmap);

  GifFileType *r = DGifOpenFileName(path, &err);
  QT_CHECK(r != NULL, "DGifOpenFileName");
  QT_CHECK(r && DGifSlurp(r) == GIF_OK, "DGifSlurp");
  QT_CHECK(r && r->SWidth == W && r->SHeight == H, "screen dimensions match");
  QT_CHECK(r && r->ImageCount == 1, "one image in the file");
  if (r) {
    SavedImage *si = &r->SavedImages[0];
    int ok = 1;
    for (int y = 0; y < H && ok; ++y)
      for (int x = 0; x < W; ++x)
        if (si->RasterBits[y * W + x] != (GifByteType)((x + y) % 4)) { ok = 0; break; }
    QT_CHECK(ok, "raster survives the round trip");
    DGifCloseFile(r, &err);
  }
  unlink(path);
  QT_END();
}
