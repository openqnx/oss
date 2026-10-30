#include "common.h"
#include <cstring>
#include <net.h>
/* ncnn: a network defined in memory -- Input -> ReLU -> 2x2 max pooling --
 * run on a known 4x4 input; the output is computed by hand. */
static const char *param =
  "7767517\n3 3\n"
  "Input   data 0 1 data 0=4 1=4 2=1\n"
  "ReLU    relu 1 1 data r\n"
  "Pooling pool 1 1 r out 0=0 1=2 2=2\n";
int main() {
  QT_BEGIN("ncnn");
  ncnn::Net net; net.opt.num_threads = 2;
  QT_CHECK(net.load_param_mem(param) == 0, "load_param_mem");
  QT_CHECK(net.load_model((const unsigned char *)"") == 0, "load_model (no weights needed)");
  const float in[16] = { -1, 2, -3, 4,   5, -6, 7, -8,   -9, -10, 11, 12,   0, 13, -14, -15 };
  ncnn::Mat m(4, 4, 1); memcpy(m.data, in, sizeof in);
  ncnn::Extractor ex = net.create_extractor();
  ex.input("data", m);
  ncnn::Mat out; QT_CHECK(ex.extract("out", out) == 0, "extract");
  printf("  info out %dx%dx%d: %g %g %g %g\n", out.w, out.h, out.c,
         out.w > 1 ? ((float *)out.data)[0] : 0.f, out.w > 1 ? ((float *)out.data)[1] : 0.f,
         out.h > 1 ? out.row(1)[0] : 0.f, out.h > 1 ? out.row(1)[1] : 0.f);
  QT_CHECK(out.w == 2 && out.h == 2 && out.c == 1, "2x2 output");
  if (out.w == 2 && out.h == 2) {
    /* max(relu) of each 2x2 block: [5 7; 13 12] */
    QT_CHECK(out.row(0)[0] == 5 && out.row(0)[1] == 7 && out.row(1)[0] == 13 && out.row(1)[1] == 12, "max pool of ReLU = [5 7; 13 12]");
  }
  QT_END();
}
