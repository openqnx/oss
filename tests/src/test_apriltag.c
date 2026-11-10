/* apriltag: render tag36h11 #42 with the library's own generator, scale it
 * up into a larger white image at a known place, and detect it: exactly one
 * detection, family tag36h11, ID 42, no corrected bits, and its centre and
 * corners where the tag was drawn. A blank image gives no detections. */
#include "common.h"
#include <math.h>
#include <apriltag/apriltag.h>
#include <apriltag/tag36h11.h>
#include <apriltag/common/image_u8.h>

int main(void) {
  QT_BEGIN("apriltag");
  apriltag_family_t *tf = tag36h11_create();
  apriltag_detector_t *td = apriltag_detector_create();
  apriltag_detector_add_family(td, tf);
  td->quad_decimate = 1.0f; td->nthreads = 2;

  image_u8_t *tag = apriltag_to_image(tf, 42);   /* 10x10 cells incl. white border */
  QT_CHECK(tag && tag->width == 10 && tag->height == 10, "tag36h11 #42 rendered as 10x10 cells");
  const int S = 20, W = 400, H = 300, X0 = 100, Y0 = 50;   /* 20 px per cell */
  image_u8_t *im = image_u8_create(W, H);
  for (int y = 0; y < H; y++) for (int x = 0; x < W; x++) im->buf[y * im->stride + x] = 255;
  for (int y = 0; y < 10 * S; y++) for (int x = 0; x < 10 * S; x++)
    im->buf[(Y0 + y) * im->stride + X0 + x] = tag->buf[(y / S) * tag->stride + x / S];

  zarray_t *dets = apriltag_detector_detect(td, im);
  QT_CHECK(zarray_size(dets) == 1, "exactly one tag detected");
  if (zarray_size(dets) >= 1) {
    apriltag_detection_t *d; zarray_get(dets, 0, &d);
    char m[128];
    QT_CHECK(d->id == 42, "its ID is 42");
    QT_CHECK(strcmp(d->family->name, "tag36h11") == 0, "its family is tag36h11");
    QT_CHECK(d->hamming == 0, "no bits corrected");
    /* The black square spans cells 1..9 of 10: pixels X0+20 .. X0+180. */
    double cx = X0 + 5 * S, cy = Y0 + 5 * S;
    snprintf(m, sizeof m, "centre at (%.1f, %.1f), expected (%.0f, %.0f)", d->c[0], d->c[1], cx, cy);
    QT_CHECK(fabs(d->c[0] - cx) < 1.5 && fabs(d->c[1] - cy) < 1.5, m);
    int corners_ok = 1;
    for (int i = 0; i < 4; i++) {
      double dx = fabs(d->p[i][0] - cx), dy = fabs(d->p[i][1] - cy);
      if (fabs(dx - 4 * S) > 2 || fabs(dy - 4 * S) > 2) corners_ok = 0;
    }
    QT_CHECK(corners_ok, "the four corners are 80 px from the centre on each axis");
  }
  apriltag_detections_destroy(dets);

  for (int i = 0; i < W * H; i++) im->buf[i] = 255;
  dets = apriltag_detector_detect(td, im);
  QT_CHECK(zarray_size(dets) == 0, "a blank image gives no detections");
  apriltag_detections_destroy(dets);

  image_u8_destroy(im); image_u8_destroy(tag);
  apriltag_detector_destroy(td); tag36h11_destroy(tf);
  QT_END();
}
