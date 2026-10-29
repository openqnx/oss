/* libpng: encode an RGBA image to memory and decode it back. */
#include "common.h"
#include <stdlib.h>
#include <png.h>
#define W 32
#define H 16
struct mem { unsigned char *p; size_t n, cap; };
static void wr(png_structp png, png_bytep data, png_size_t len) {
  struct mem *m = png_get_io_ptr(png);
  if (m->n + len > m->cap) { m->cap = (m->n + len) * 2; m->p = realloc(m->p, m->cap); }
  memcpy(m->p + m->n, data, len); m->n += len;
}
static void fl(png_structp png) { (void)png; }
struct rd { const unsigned char *p; size_t n, off; };
static void rdfn(png_structp png, png_bytep data, png_size_t len) {
  struct rd *r = png_get_io_ptr(png);
  memcpy(data, r->p + r->off, len); r->off += len;
}
int main(void) {
  QT_BEGIN("libpng");
  printf("  info libpng %s\n", png_get_libpng_ver(NULL));
  unsigned char img[H][W][4];
  for (int y = 0; y < H; ++y) for (int x = 0; x < W; ++x) {
    img[y][x][0] = (unsigned char)(x * 8); img[y][x][1] = (unsigned char)(y * 16);
    img[y][x][2] = 0x40; img[y][x][3] = 0xFF;
  }
  struct mem m = { malloc(4096), 0, 4096 };
  png_structp png = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
  png_infop info = png_create_info_struct(png);
  QT_CHECK(png && info, "create write structs");
  png_set_write_fn(png, &m, wr, fl);
  png_set_IHDR(png, info, W, H, 8, PNG_COLOR_TYPE_RGBA, PNG_INTERLACE_NONE,
               PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);
  png_write_info(png, info);
  png_bytep rows[H];
  for (int y = 0; y < H; ++y) rows[y] = img[y][0];
  png_write_image(png, rows);
  png_write_end(png, NULL);
  png_destroy_write_struct(&png, &info);
  QT_CHECK(m.n > 8, "PNG bytes written");
  QT_CHECK(png_sig_cmp(m.p, 0, 8) == 0, "output starts with the PNG signature");

  struct rd r = { m.p, m.n, 0 };
  png_structp rp = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
  png_infop ri = png_create_info_struct(rp);
  png_set_read_fn(rp, &r, rdfn);
  png_read_info(rp, ri);
  QT_CHECK(png_get_image_width(rp, ri) == W && png_get_image_height(rp, ri) == H,
           "decoded dimensions match");
  QT_CHECK(png_get_color_type(rp, ri) == PNG_COLOR_TYPE_RGBA, "colour type is RGBA");
  unsigned char back[H][W][4];
  png_bytep brows[H];
  for (int y = 0; y < H; ++y) brows[y] = back[y][0];
  png_read_image(rp, brows);
  png_read_end(rp, NULL);
  png_destroy_read_struct(&rp, &ri, NULL);
  QT_CHECK(memcmp(img, back, sizeof img) == 0, "pixels survive the round trip");
  free(m.p);
  QT_END();
}
