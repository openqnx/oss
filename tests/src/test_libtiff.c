/* libtiff: write a strip-based RGB TIFF to a file and read it back. */
#include "common.h"
#include <stdlib.h>
#include <unistd.h>
#include <tiffio.h>
#define W 40
#define H 20
int main(void) {
  QT_BEGIN("libtiff");
  printf("  info %s", TIFFGetVersion());
  char path[] = "/tmp/qnxport-tif-XXXXXX";
  int fd = mkstemp(path);
  QT_CHECK(fd >= 0, "create temp file");
  if (fd >= 0) close(fd);
  unsigned char img[H][W][3];
  for (int y = 0; y < H; ++y) for (int x = 0; x < W; ++x) {
    img[y][x][0] = (unsigned char)(x * 6); img[y][x][1] = (unsigned char)(y * 12);
    img[y][x][2] = 0x55;
  }
  TIFF *t = TIFFOpen(path, "w");
  QT_CHECK(t != NULL, "TIFFOpen for write");
  TIFFSetField(t, TIFFTAG_IMAGEWIDTH, W);
  TIFFSetField(t, TIFFTAG_IMAGELENGTH, H);
  TIFFSetField(t, TIFFTAG_SAMPLESPERPIXEL, 3);
  TIFFSetField(t, TIFFTAG_BITSPERSAMPLE, 8);
  TIFFSetField(t, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_RGB);
  TIFFSetField(t, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG);
  TIFFSetField(t, TIFFTAG_COMPRESSION, COMPRESSION_DEFLATE);
  TIFFSetField(t, TIFFTAG_ROWSPERSTRIP, TIFFDefaultStripSize(t, 0));
  int wrote = 1;
  for (int y = 0; y < H; ++y)
    if (TIFFWriteScanline(t, img[y][0], (uint32_t)y, 0) < 0) wrote = 0;
  QT_CHECK(wrote, "TIFFWriteScanline for every row");
  TIFFClose(t);

  t = TIFFOpen(path, "r");
  QT_CHECK(t != NULL, "TIFFOpen for read");
  uint32_t w = 0, h = 0; uint16_t spp = 0, bps = 0, comp = 0;
  TIFFGetField(t, TIFFTAG_IMAGEWIDTH, &w);
  TIFFGetField(t, TIFFTAG_IMAGELENGTH, &h);
  TIFFGetField(t, TIFFTAG_SAMPLESPERPIXEL, &spp);
  TIFFGetField(t, TIFFTAG_BITSPERSAMPLE, &bps);
  TIFFGetField(t, TIFFTAG_COMPRESSION, &comp);
  QT_CHECK(w == W && h == H, "dimensions match");
  QT_CHECK(spp == 3 && bps == 8, "3 samples of 8 bits");
  QT_CHECK(comp == COMPRESSION_DEFLATE, "deflate compression survives (zlib is wired up)");
  unsigned char back[H][W][3];
  int ok = 1;
  for (int y = 0; y < H; ++y)
    if (TIFFReadScanline(t, back[y][0], (uint32_t)y, 0) < 0) ok = 0;
  QT_CHECK(ok, "TIFFReadScanline for every row");
  QT_CHECK(memcmp(img, back, sizeof img) == 0, "pixels survive the round trip");
  TIFFClose(t);
  unlink(path);
  QT_END();
}
