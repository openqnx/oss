/* libyuv: colour conversion and scaling, including the NEON/SSE row functions
 * selected at run time -- the aarch64 inline assembly in row_neon64.cc and
 * scale_neon64.cc had to be reworked for the SDP assembler, so exercising a
 * real conversion is what proves that rework is correct. */
#include "common.h"
#include <vector>
#include <libyuv.h>
#define W 64
#define H 32
int main() {
  QT_BEGIN("libyuv");
  std::vector<uint8_t> argb(W * H * 4), back(W * H * 4);
  std::vector<uint8_t> y(W * H), u(W * H / 4), v(W * H / 4);
  for (int i = 0; i < W * H; ++i) {
    /* A smooth gradient: 4:2:0 subsampling averages neighbouring chroma, so a
     * high-frequency pattern would lose colour detail for reasons that have
     * nothing to do with this port. */
    const int x = i % W, y = i / W;
    argb[i * 4 + 0] = uint8_t(x * 255 / (W - 1));          /* B */
    argb[i * 4 + 1] = uint8_t(y * 255 / (H - 1));          /* G */
    argb[i * 4 + 2] = uint8_t((x + y) * 255 / (W + H - 2));/* R */
    argb[i * 4 + 3] = 255;                   /* A */
  }
  int rc = libyuv::ARGBToI420(argb.data(), W * 4, y.data(), W,
                              u.data(), W / 2, v.data(), W / 2, W, H);
  QT_CHECK(rc == 0, "ARGBToI420");
  rc = libyuv::I420ToARGB(y.data(), W, u.data(), W / 2, v.data(), W / 2,
                          back.data(), W * 4, W, H);
  QT_CHECK(rc == 0, "I420ToARGB");
  long diff = 0;
  for (int i = 0; i < W * H; ++i)
    for (int c = 0; c < 3; ++c)
      diff += labs(long(argb[i * 4 + c]) - long(back[i * 4 + c]));
  /* 4:2:0 subsampling is lossy; the average error must still be small. */
  QT_CHECK(diff / (W * H * 3) < 20, "ARGB -> I420 -> ARGB stays close");

  std::vector<uint8_t> half(W / 2 * H / 2);
  /* ScalePlane and MirrorPlane return void -- calling them and checking the
   * pixels they produced is the only way to verify them. */
  libyuv::ScalePlane(y.data(), W, W, H, half.data(), W / 2, W / 2, H / 2,
                     libyuv::kFilterBilinear);
  long sum_full = 0, sum_half = 0;
  for (int i = 0; i < W * H; ++i) sum_full += y[i];
  for (int i = 0; i < W / 2 * H / 2; ++i) sum_half += half[i];
  QT_CHECK(labs(sum_full / (W * H) - sum_half / (W / 2 * H / 2)) < 6,
           "mean luma is preserved by scaling");

  std::vector<uint8_t> mirrored(W * H);
  libyuv::MirrorPlane(y.data(), W, mirrored.data(), W, W, H);
  QT_CHECK(mirrored[0] == y[W - 1] && mirrored[W - 1] == y[0], "row was mirrored");

  std::vector<uint8_t> rot(H * W);
  rc = libyuv::RotatePlane(y.data(), W, rot.data(), H, W, H, libyuv::kRotate90);
  QT_CHECK(rc == 0, "RotatePlane 90 degrees");
  QT_CHECK(rot[0] == y[(H - 1) * W], "rotation moved the bottom-left pixel to the top-left");
  QT_END();
}
