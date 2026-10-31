/* XNNPACK: initialise (this needs cpuinfo to recognise the CPU), then run
 * f32 operators with known outputs: broadcasting add and multiply, a fully
 * connected layer (with bias and a ReLU clamp) on a 2-thread pthreadpool,
 * and a square-root unary op. */
#include "common.h"
#include <math.h>
#include <xnnpack.h>
#include <pthreadpool.h>
int main(void) {
  QT_BEGIN("xnnpack");
  enum xnn_status st = xnn_initialize(NULL);
  printf("  info xnn_initialize -> %d\n", (int)st);
  QT_CHECK(st == xnn_status_success, "xnn_initialize (CPU recognised)");
  if (st != xnn_status_success) QT_END();
  pthreadpool_t tp = pthreadpool_create(2);

  float a[6] = { 1, 2, 3, 4, 5, 6 }, b[3] = { 10, 20, 30 }, o[6] = { 0 };
  size_t sa[2] = { 2, 3 }, sb[1] = { 3 };
  st = xnn_run_binary_elementwise_nd(xnn_binary_add, xnn_datatype_fp32, NULL, NULL, NULL, 0, 2, sa, 1, sb, a, b, o, tp);
  printf("  info add -> %g %g %g %g %g %g\n", o[0], o[1], o[2], o[3], o[4], o[5]);
  QT_CHECK(st == xnn_status_success && o[0] == 11 && o[2] == 33 && o[3] == 14 && o[5] == 36, "add [2,3] + [3] (broadcast)");
  st = xnn_run_binary_elementwise_nd(xnn_binary_multiply, xnn_datatype_fp32, NULL, NULL, NULL, 0, 2, sa, 1, sb, a, b, o, tp);
  QT_CHECK(st == xnn_status_success && o[0] == 10 && o[1] == 40 && o[5] == 180, "multiply [2,3] * [3] (broadcast)");

  /* y = relu(W x + bias): W 2x3 (output_channels x input_channels) */
  float W[6] = { 1, 0, -1, 0.5f, 0.5f, 0.5f }, bias[2] = { 0.25f, -10 };
  float x[2 * 3] = { 3, 1, 2, 6, 8, 10 }, y[2 * 2] = { -1, -1, -1, -1 };
  xnn_operator_t fc = NULL;
  st = xnn_create_fully_connected_nc_f32(3, 2, 3, 2, W, bias, 0.0f, INFINITY, 0, NULL, NULL, &fc);
  QT_CHECK(st == xnn_status_success && fc, "create fully_connected f32");
  if (fc) {
    st = xnn_reshape_fully_connected_nc_f32(fc, 2, tp);
    if (st == xnn_status_success) st = xnn_setup_fully_connected_nc_f32(fc, x, y);
    if (st == xnn_status_success) st = xnn_run_operator(fc, tp);
    printf("  info fc -> %g %g %g %g\n", y[0], y[1], y[2], y[3]);
    /* row 0: [3-2+0.25, 3-10] -> [1.25, 0 (relu)]; row 1: [6-10+0.25 -> 0, 12-10 = 2] */
    QT_CHECK(st == xnn_status_success && y[0] == 1.25f && y[1] == 0 && y[2] == 0 && y[3] == 2, "fully connected + bias + ReLU");
    xnn_delete_operator(fc);
  }
  float sq[5] = { 0, 1, 4, 9, 2.25f }, r[5] = { -1 };
  st = xnn_run_unary_elementwise_nc(xnn_unary_square_root, xnn_datatype_fp32, xnn_datatype_fp32, NULL, NULL, NULL, 0,
                                    1, 5, 5, 5, tp, sq, r);
  printf("  info sqrt -> status %d: %.9g %.9g %.9g %.9g %.9g\n", (int)st, r[0], r[1], r[2], r[3], r[4]);
  /* XNNPACK's f32 sqrt kernels may use a reciprocal-sqrt estimate plus a
     Newton step (its x86 AVX/AVX-512 variants do): within a few ulp, not
     correctly rounded. */
  int sq_ok = st == xnn_status_success && r[0] == 0;
  for (int i = 1; i < 5; i++) sq_ok = sq_ok && fabsf(r[i] - sqrtf(sq[i])) <= 1e-6f * sqrtf(sq[i]);
  QT_CHECK(sq_ok, "square root unary op (within 1e-6 relative)");
  pthreadpool_destroy(tp);
  QT_CHECK(xnn_deinitialize() == xnn_status_success, "xnn_deinitialize");
  QT_END();
}
