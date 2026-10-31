/* libjpeg-turbo: the TurboJPEG 3 API and the classic libjpeg API on a known
 * gradient image, then the programs on the same picture: cjpeg/djpeg round
 * trip, jpegtran rotation (dimensions swap), wrjpgcom/rdjpgcom comment. */
#include "common.h"
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <turbojpeg.h>
#include <jpeglib.h>

#define W 64
#define H 48
static char dir[64];

static int run(const char *cmd) {
  char full[1024];
  snprintf(full, sizeof full, "cd %s && %s", dir, cmd);
  int rc = system(full);
  printf("  info $ %s -> %d\n", cmd, rc);
  return rc;
}
static unsigned char *slurp(const char *name, size_t *len) {
  char p[256]; snprintf(p, sizeof p, "%s/%s", dir, name);
  FILE *f = fopen(p, "rb"); if (!f) return NULL;
  fseek(f, 0, SEEK_END); long n = ftell(f); fseek(f, 0, SEEK_SET);
  unsigned char *b = malloc(n > 0 ? n : 1);
  if (fread(b, 1, n, f) != (size_t)n) { free(b); fclose(f); return NULL; }
  fclose(f); *len = n; return b;
}
static double mean_abs(const unsigned char *a, const unsigned char *b, size_t n) {
  double s = 0; for (size_t i = 0; i < n; i++) s += abs((int)a[i] - (int)b[i]);
  return s / n;
}
/* Parse a binary P6 PPM; returns pointer to pixel data inside buf. */
static unsigned char *ppm_pixels(unsigned char *buf, size_t len, int *w, int *h) {
  int maxv = 0, off = 0;
  if (len < 16 || sscanf((char *)buf, "P6 %d %d %d%n", w, h, &maxv, &off) != 3 || maxv != 255) return NULL;
  off++;  /* the single whitespace after maxval */
  if ((size_t)off + (size_t)(*w) * (*h) * 3 > len) return NULL;
  return buf + off;
}

int main(void) {
  QT_BEGIN("libjpeg-turbo");
  strcpy(dir, "/tmp/qnxport-jpeg.XXXXXX");
  QT_CHECK(mkdtemp(dir) != NULL, "scratch directory");

  static unsigned char img[W * H * 3];
  for (int y = 0; y < H; y++)
    for (int x = 0; x < W; x++) {
      unsigned char *p = img + (y * W + x) * 3;
      p[0] = (unsigned char)(x * 4); p[1] = (unsigned char)(y * 5); p[2] = 128;
    }

  /* ---- TurboJPEG 3 ---- */
  tjhandle tc = tj3Init(TJINIT_COMPRESS);
  QT_CHECK(tc != NULL, "tj3Init(COMPRESS)");
  tj3Set(tc, TJPARAM_QUALITY, 95);
  tj3Set(tc, TJPARAM_SUBSAMP, TJSAMP_444);
  unsigned char *jpg = NULL; size_t jsz = 0;
  int rc = tj3Compress8(tc, img, W, 0, H, TJPF_RGB, &jpg, &jsz);
  QT_CHECK(rc == 0 && jsz > 100, "tj3Compress8");
  printf("  info %dx%d RGB -> %zu byte JPEG\n", W, H, jsz);
  QT_CHECK(jpg && jpg[0] == 0xFF && jpg[1] == 0xD8 && jpg[jsz - 2] == 0xFF && jpg[jsz - 1] == 0xD9,
           "SOI/EOI markers");
  tjhandle td = tj3Init(TJINIT_DECOMPRESS);
  QT_CHECK(td && tj3DecompressHeader(td, jpg, jsz) == 0, "tj3DecompressHeader");
  QT_CHECK(tj3Get(td, TJPARAM_JPEGWIDTH) == W && tj3Get(td, TJPARAM_JPEGHEIGHT) == H, "header says 64x48");
  QT_CHECK(tj3Get(td, TJPARAM_SUBSAMP) == TJSAMP_444, "header says 4:4:4");
  static unsigned char back[W * H * 3];
  QT_CHECK(tj3Decompress8(td, jpg, jsz, back, 0, TJPF_RGB) == 0, "tj3Decompress8");
  double d = mean_abs(img, back, sizeof img);
  printf("  info mean |error| after q95 round trip: %.3f\n", d);
  QT_CHECK(d < 2.0, "decoded pixels match the original (mean error < 2)");

  /* ---- classic libjpeg API on the same JPEG ---- */
  struct jpeg_decompress_struct ci; struct jpeg_error_mgr jerr;
  ci.err = jpeg_std_error(&jerr);
  jpeg_create_decompress(&ci);
  jpeg_mem_src(&ci, jpg, (unsigned long)jsz);
  QT_CHECK(jpeg_read_header(&ci, TRUE) == JPEG_HEADER_OK, "jpeg_read_header");
  ci.out_color_space = JCS_RGB;
  jpeg_start_decompress(&ci);
  QT_CHECK(ci.output_width == W && ci.output_height == H && ci.output_components == 3, "libjpeg: 64x48x3");
  static unsigned char back2[W * H * 3];
  while (ci.output_scanline < ci.output_height) {
    JSAMPROW row = back2 + ci.output_scanline * W * 3;
    jpeg_read_scanlines(&ci, &row, 1);
  }
  jpeg_finish_decompress(&ci);
  jpeg_destroy_decompress(&ci);
  QT_CHECK(mean_abs(back, back2, sizeof back) < 1.0, "libjpeg and TurboJPEG decode alike");

  /* ---- the programs ---- */
  char p[256]; snprintf(p, sizeof p, "%s/in.ppm", dir);
  FILE *f = fopen(p, "wb");
  if (f) { fprintf(f, "P6\n%d %d\n255\n", W, H); fwrite(img, 1, sizeof img, f); fclose(f); }
  QT_CHECK(run("cjpeg -quality 95 -sample 1x1 -outfile a.jpg in.ppm") == 0, "cjpeg PPM -> JPEG");
  size_t alen = 0; unsigned char *a = slurp("a.jpg", &alen);
  QT_CHECK(a && alen > 100 && a[0] == 0xFF && a[1] == 0xD8, "cjpeg wrote a JPEG");
  if (a) {
    QT_CHECK(tj3DecompressHeader(td, a, alen) == 0 && tj3Get(td, TJPARAM_JPEGWIDTH) == W &&
             tj3Get(td, TJPARAM_JPEGHEIGHT) == H, "cjpeg output is 64x48 (read with TurboJPEG)");
    free(a);
  }
  QT_CHECK(run("djpeg -pnm -outfile out.ppm a.jpg") == 0, "djpeg JPEG -> PPM");
  size_t olen = 0; unsigned char *o = slurp("out.ppm", &olen);
  int ow = 0, oh = 0; unsigned char *px = o ? ppm_pixels(o, olen, &ow, &oh) : NULL;
  QT_CHECK(px && ow == W && oh == H, "djpeg output is a 64x48 PPM");
  if (px && ow == W && oh == H) {
    double e = mean_abs(img, px, sizeof img);
    printf("  info cjpeg/djpeg mean |error|: %.3f\n", e);
    QT_CHECK(e < 2.0, "cjpeg/djpeg round trip preserves the picture");
  }
  free(o);
  QT_CHECK(run("jpegtran -rotate 90 -perfect -outfile r.jpg a.jpg") == 0, "jpegtran -rotate 90");
  size_t rlen = 0; unsigned char *r = slurp("r.jpg", &rlen);
  QT_CHECK(r && tj3DecompressHeader(td, r, rlen) == 0 && tj3Get(td, TJPARAM_JPEGWIDTH) == H &&
           tj3Get(td, TJPARAM_JPEGHEIGHT) == W, "rotated JPEG is 48x64");
  free(r);
  QT_CHECK(run("wrjpgcom -comment 'QNX jpeg comment 42' a.jpg > c.jpg") == 0, "wrjpgcom");
  QT_CHECK(run("rdjpgcom c.jpg > c.txt") == 0, "rdjpgcom");
  size_t clen = 0; unsigned char *c = slurp("c.txt", &clen);
  if (c) { c = realloc(c, clen + 1); c[clen] = 0; }
  QT_CHECK(c && strstr((char *)c, "QNX jpeg comment 42"), "rdjpgcom reads back the comment");
  free(c);

  tj3Free(jpg); tj3Destroy(tc); tj3Destroy(td);
  char rm[128]; snprintf(rm, sizeof rm, "rm -rf %s", dir); system(rm);
  QT_END();
}
