/* gd (libgd): draw on a true-colour image -- filled rectangle, line,
 * ellipse, flood fill -- and check pixels at known places; write it as PNG,
 * JPEG, WebP, TIFF, GIF and BMP to memory, read each back and check size and
 * pixels (exact for the lossless formats; GIF is a quantized palette); scale and rotate it; and render
 * text with FreeType when a font is installed (DejaVu). */
#include "common.h"
#include <gd.h>
#include <stdlib.h>
#include <unistd.h>

static int rgb(gdImagePtr im, int x, int y) {
  int c = gdImageGetTrueColorPixel(im, x, y);
  return (gdTrueColorGetRed(c) << 16) | (gdTrueColorGetGreen(c) << 8) | gdTrueColorGetBlue(c);
}

int main(void) {
  QT_BEGIN("gd");
  printf("  info gd %s\n", GD_VERSION_STRING);
  gdImagePtr im = gdImageCreateTrueColor(100, 80);
  QT_CHECK(im && gdImageSX(im) == 100 && gdImageSY(im) == 80, "100x80 true-colour image");
  int white = gdImageColorAllocate(im, 255, 255, 255), red = gdImageColorAllocate(im, 255, 0, 0);
  int blue = gdImageColorAllocate(im, 0, 0, 255), green = gdImageColorAllocate(im, 0, 255, 0);
  gdImageFilledRectangle(im, 0, 0, 99, 79, white);
  gdImageFilledRectangle(im, 10, 10, 29, 29, red);
  gdImageLine(im, 0, 50, 99, 50, blue);
  gdImageEllipse(im, 70, 25, 30, 30, green);
  gdImageFill(im, 70, 25, blue);   /* inside the ellipse */
  QT_CHECK(rgb(im, 20, 20) == 0xff0000 && rgb(im, 5, 5) == 0xffffff, "filled rectangle: red inside, white outside");
  QT_CHECK(rgb(im, 0, 50) == 0x0000ff && rgb(im, 99, 50) == 0x0000ff && rgb(im, 50, 51) == 0xffffff, "line along y=50");
  QT_CHECK(rgb(im, 70, 25) == 0x0000ff && rgb(im, 70, 10) == 0x00ff00 && rgb(im, 90, 25) == 0xffffff, "ellipse outline green, flood-filled blue inside");

  struct { const char *n; void *(*save)(gdImagePtr, int *); gdImagePtr (*load)(int, void *); int exact; } fm[] = {
    { "PNG",  gdImagePngPtr,  gdImageCreateFromPngPtr,  1 },
    { "GIF",  gdImageGifPtr,  gdImageCreateFromGifPtr,  0 },   /* palette: colours quantized */
    { "BMP",  NULL,           gdImageCreateFromBmpPtr,  1 },
    { "TIFF", gdImageTiffPtr, gdImageCreateFromTiffPtr, 1 },
    { "WebP", gdImageWebpPtr, gdImageCreateFromWebpPtr, 0 },
    { "JPEG", NULL,           gdImageCreateFromJpegPtr, 0 },
  };
  for (unsigned i = 0; i < sizeof fm / sizeof fm[0]; i++) {
    int size = 0; void *data;
    if (!strcmp(fm[i].n, "BMP")) data = gdImageBmpPtr(im, &size, 0);
    else if (!strcmp(fm[i].n, "JPEG")) data = gdImageJpegPtr(im, &size, 95);
    else data = fm[i].save(im, &size);
    gdImagePtr back = data ? fm[i].load(size, data) : NULL;
    int ok = back && gdImageSX(back) == 100 && gdImageSY(back) == 80;
    if (ok && fm[i].exact) {
      if (!gdImageTrueColor(back)) { gdImagePaletteToTrueColor(back); }
      ok = rgb(back, 20, 20) == 0xff0000 && rgb(back, 70, 25) == 0x0000ff && rgb(back, 5, 5) == 0xffffff;
    } else if (ok) {
      int c = rgb(back, 20, 20);
      ok = (c >> 16) > 230 && ((c >> 8) & 0xff) < 30 && (c & 0xff) < 30;
    }
    char what[96]; snprintf(what, sizeof what, "%s: written (%d bytes) and read back%s", fm[i].n, size, fm[i].exact ? " exactly" : " (lossy, red stays red)");
    QT_CHECK(ok, what);
    if (back) gdImageDestroy(back);
    if (data) gdFree(data);
  }

  gdImagePtr sc = gdImageScale(im, 50, 40);
  QT_CHECK(sc && gdImageSX(sc) == 50 && gdImageSY(sc) == 40 && (rgb(sc, 10, 10) >> 16) > 200, "scaled to 50x40, the red square still red");
  if (sc) gdImageDestroy(sc);
  gdImagePtr rot = gdImageRotateInterpolated(im, 90, white);
  QT_CHECK(rot && gdImageSX(rot) == 80 && gdImageSY(rot) == 100, "rotated 90 degrees: 80x100");
  if (rot) gdImageDestroy(rot);

  const char *fonts[] = { "/usr/local/share/fonts/font-dejavu/DejaVuSans.ttf",
                          "/data/oss/usr/local/share/fonts/font-dejavu/DejaVuSans.ttf" };
  const char *font = NULL;
  for (int i = 0; i < 2; i++) if (access(fonts[i], R_OK) == 0) font = fonts[i];
  QT_CHECK(gdFTUseFontConfig(1) == 1, "built with fontconfig");
  if (font) {
    int br[8];
    gdImageFilledRectangle(im, 0, 55, 99, 79, white);
    char *err = gdImageStringFT(im, br, 0 /* black, antialiased */, (char *)font, 14, 0, 4, 75, "Hello");
    if (err) printf("  info %s\n", err);
    int dark = 0;
    for (int y = 55; y < 80; y++) for (int x = 0; x < 100; x++) if (rgb(im, x, y) < 0x404040) dark++;
    QT_CHECK(!err && br[2] - br[0] > 30 && br[2] - br[0] < 70 && dark > 40, "FreeType text 'Hello' at 14 pt: 30-70 px wide, drawn");
  } else {
    printf("  info no DejaVu font installed: FreeType text not checked\n");
  }
  gdImageDestroy(im);
  QT_END();
}
