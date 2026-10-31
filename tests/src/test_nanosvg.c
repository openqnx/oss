/* nanosvg: parse an SVG document (a red square, a blue circle, a path with
 * a group transform) into shapes and paths, then rasterise it with
 * nanosvgrast and check pixels at 1x and 2x scale. Built against the
 * package's static libraries, not the single-header implementation. */
#include "common.h"
#include <math.h>
#include <stdlib.h>
#include <nanosvg.h>
#include <nanosvgrast.h>

static const char *svg =
  "<svg xmlns='http://www.w3.org/2000/svg' width='40' height='20' viewBox='0 0 40 20'>"
  "<rect x='0' y='0' width='20' height='20' fill='#ff0000'/>"
  "<circle cx='30' cy='10' r='8' fill='rgb(0,0,255)'/>"
  "<g transform='translate(20,0)'><path d='M0 0 L4 0 L4 4 Z' fill='#00ff00' stroke='#000' stroke-width='0.5'/></g>"
  "</svg>";

static const unsigned char *px(const unsigned char *img, int w, int x, int y) { return img + (y * w + x) * 4; }

int main(void) {
  QT_BEGIN("nanosvg");
  char *buf = strdup(svg);                      /* nsvgParse writes into its input */
  NSVGimage *im = nsvgParse(buf, "px", 96.0f);
  QT_CHECK(im != NULL, "nsvgParse");
  if (!im) QT_END();
  QT_CHECK(im->width == 40.0f && im->height == 20.0f, "image is 40x20");
  int n = 0, npaths = 0;
  NSVGshape *sh[8];
  for (NSVGshape *s = im->shapes; s && n < 8; s = s->next) {
    sh[n++] = s;
    for (NSVGpath *p = s->paths; p; p = p->next) npaths++;
  }
  printf("  info %d shapes, %d paths\n", n, npaths);
  QT_CHECK(n == 3, "three shapes");
  if (n == 3) {
    QT_CHECK(sh[0]->fill.type == NSVG_PAINT_COLOR && sh[0]->fill.color == 0xFF0000FFu, "rect fill red (ABGR 0xFF0000FF)");
    QT_CHECK(sh[1]->fill.color == 0xFFFF0000u, "circle fill rgb(0,0,255)");
    QT_CHECK(fabsf(sh[1]->bounds[0] - 22) < 0.01f && fabsf(sh[1]->bounds[2] - 38) < 0.01f, "circle bounds x 22..38");
    QT_CHECK(fabsf(sh[2]->bounds[0] - 20) < 0.01f && fabsf(sh[2]->bounds[2] - 24) < 0.01f, "path translated by the group to x 20..24");
    QT_CHECK(sh[2]->stroke.type == NSVG_PAINT_COLOR && fabsf(sh[2]->strokeWidth - 0.5f) < 1e-4f, "path stroke, width 0.5");
    QT_CHECK(sh[2]->paths && sh[2]->paths->closed, "path closed (Z)");
  }
  NSVGrasterizer *r = nsvgCreateRasterizer();
  QT_CHECK(r != NULL, "nsvgCreateRasterizer");
  unsigned char *img = calloc(40 * 20, 4);
  nsvgRasterize(r, im, 0, 0, 1.0f, img, 40, 20, 40 * 4);
  const unsigned char *a = px(img, 40, 10, 10), *b = px(img, 40, 30, 10), *c = px(img, 40, 39, 0), *g = px(img, 40, 22, 1);
  printf("  info (10,10)=%d,%d,%d,%d (30,10)=%d,%d,%d,%d (39,0)=%d\n", a[0], a[1], a[2], a[3], b[0], b[1], b[2], b[3], c[3]);
  QT_CHECK(a[0] == 255 && a[1] == 0 && a[2] == 0 && a[3] == 255, "pixel (10,10) is red");
  QT_CHECK(b[0] == 0 && b[1] == 0 && b[2] == 255 && b[3] == 255, "pixel (30,10) is blue");
  QT_CHECK(c[3] == 0, "pixel (39,0) outside every shape is transparent");
  QT_CHECK(g[1] > 200 && g[0] < 60, "pixel (22,1) inside the triangle is green");
  long red = 0;
  for (int y = 0; y < 20; y++)      /* column 19 touches the stroked triangle: anti-aliased */
    for (int x = 0; x < 19; x++) { const unsigned char *q = px(img, 40, x, y); red += q[0] == 255 && q[1] == 0 && q[3] == 255; }
  QT_CHECK(red == 19 * 20, "the square's pixels (columns 0-18) are all solid red");
  unsigned char *img2 = calloc(80 * 40, 4);
  nsvgRasterize(r, im, 0, 0, 2.0f, img2, 80, 40, 80 * 4);
  const unsigned char *d = px(img2, 80, 60, 20), *e = px(img2, 80, 79, 39);
  QT_CHECK(d[2] == 255 && d[3] == 255 && e[3] == 0, "2x: (60,20) blue, (79,39) transparent");
  free(img); free(img2);
  nsvgDeleteRasterizer(r);
  nsvgDelete(im);
  free(buf);
  QT_END();
}
