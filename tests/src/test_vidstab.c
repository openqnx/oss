/* vid.stab: the video stabiliser ffmpeg uses for vidstabdetect/vidstabtransform.
 * Motion detection is pure computation, so it runs headless: two synthetic
 * grayscale frames, the second shifted, are fed through a real detector. */
#include "common.h"
#include <stdlib.h>
#include <vid.stab/libvidstab.h>
#define W 128
#define H 96
int main(void) {
  QT_BEGIN("vidstab");
  VSFrameInfo fi;
  /* vsFrameInfoInit does not use the VS_OK convention that the motiondetect
     entry points document -- it returns non-zero when the pixel format is
     supported -- so check the fields it filled in rather than the code. */
  QT_CHECK(vsFrameInfoInit(&fi, W, H, PF_GRAY8) != 0, "vsFrameInfoInit");
  QT_CHECK(fi.width == W && fi.height == H, "frame info records the geometry");

  VSMotionDetectConfig conf = vsMotionDetectGetDefaultConfig("qnx-port-test");
  VSMotionDetect md;
  QT_CHECK(vsMotionDetectInit(&md, &conf, &fi) == VS_OK, "vsMotionDetectInit");

  VSFrame frame;
  vsFrameAllocate(&frame, &fi);
  QT_CHECK(frame.data[0] != NULL, "vsFrameAllocate");

  /* Two passes over a frame containing a bright block; the block moves by 8
     pixels between them, which is what the detector is asked to find. */
  int ok = 1;
  for (int pass = 0; pass < 2 && ok; ++pass) {
    int dx = pass * 8;
    for (int y = 0; y < H; ++y)
      for (int x = 0; x < W; ++x)
        frame.data[0][y * frame.linesize[0] + x] =
          (x > 30 + dx && x < 70 + dx && y > 20 && y < 60) ? 230 : 20;
    LocalMotions motions;
    if (vsMotionDetection(&md, &motions, &frame) != VS_OK) ok = 0;
    else vs_vector_del(&motions);
  }
  QT_CHECK(ok, "vsMotionDetection over two frames");

  vsFrameFree(&frame);
  vsMotionDetectionCleanup(&md);
  QT_CHECK(1, "cleanup");
  QT_END();
}
