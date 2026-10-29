/* libjpeg-turbo: compress an RGB image to memory and decompress it back. */
#include "common.h"
#include <stdlib.h>
#include <stdio.h>
#include <jpeglib.h>
#define W 64
#define H 32
int main(void) {
  QT_BEGIN("libjpeg-turbo");
  unsigned char img[H][W][3];
  for (int y = 0; y < H; ++y) for (int x = 0; x < W; ++x) {
    img[y][x][0] = (unsigned char)(x * 4); img[y][x][1] = (unsigned char)(y * 8);
    img[y][x][2] = 0x30;
  }
  struct jpeg_compress_struct cinfo;
  struct jpeg_error_mgr jerr;
  cinfo.err = jpeg_std_error(&jerr);
  jpeg_create_compress(&cinfo);
  unsigned char *buf = NULL; unsigned long blen = 0;
  jpeg_mem_dest(&cinfo, &buf, &blen);
  cinfo.image_width = W; cinfo.image_height = H;
  cinfo.input_components = 3; cinfo.in_color_space = JCS_RGB;
  jpeg_set_defaults(&cinfo);
  jpeg_set_quality(&cinfo, 90, TRUE);
  jpeg_start_compress(&cinfo, TRUE);
  while (cinfo.next_scanline < cinfo.image_height) {
    JSAMPROW row = img[cinfo.next_scanline][0];
    jpeg_write_scanlines(&cinfo, &row, 1);
  }
  jpeg_finish_compress(&cinfo);
  jpeg_destroy_compress(&cinfo);
  QT_CHECK(buf && blen > 100, "JPEG bytes produced");
  QT_CHECK(buf && buf[0] == 0xFF && buf[1] == 0xD8, "output starts with SOI marker");

  struct jpeg_decompress_struct dinfo;
  dinfo.err = jpeg_std_error(&jerr);
  jpeg_create_decompress(&dinfo);
  jpeg_mem_src(&dinfo, buf, blen);
  QT_CHECK(jpeg_read_header(&dinfo, TRUE) == JPEG_HEADER_OK, "jpeg_read_header");
  jpeg_start_decompress(&dinfo);
  QT_CHECK(dinfo.output_width == W && dinfo.output_height == H,
           "decoded dimensions match");
  QT_CHECK(dinfo.output_components == 3, "three colour components");
  unsigned char row[W * 3];
  long diff = 0; int rows = 0;
  while (dinfo.output_scanline < dinfo.output_height) {
    JSAMPROW r = row;
    int y = (int)dinfo.output_scanline;
    jpeg_read_scanlines(&dinfo, &r, 1);
    for (int i = 0; i < W * 3; ++i) diff += labs((long)row[i] - (long)img[y][0][i]);
    ++rows;
  }
  jpeg_finish_decompress(&dinfo);
  jpeg_destroy_decompress(&dinfo);
  QT_CHECK(rows == H, "all scanlines decoded");
  /* Lossy, so allow a small average error rather than an exact match. */
  QT_CHECK(diff / (W * H * 3) < 8, "decoded pixels are close to the original");
  free(buf);
  QT_END();
}
