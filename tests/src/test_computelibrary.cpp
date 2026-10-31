/* Arm Compute Library: NEON kernels on CPU tensors with known results --
 * element-wise addition, a 2x3 * 3x2 GEMM, ReLU activation, 2x2 max
 * pooling and a transpose, on the default (multi-threaded) CPU scheduler.
 * aarch64 only, like the port. */
#include "common.h"
#include <cmath>
#include <vector>
#include "arm_compute/core/Types.h"
#include "arm_compute/core/Version.h"
#include "arm_compute/runtime/Tensor.h"
#include "arm_compute/runtime/Scheduler.h"
#include "arm_compute/runtime/NEON/functions/NEArithmeticAddition.h"
#include "arm_compute/runtime/NEON/functions/NEGEMM.h"
#include "arm_compute/runtime/NEON/functions/NEActivationLayer.h"
#include "arm_compute/runtime/NEON/functions/NEPoolingLayer.h"
#include "arm_compute/runtime/NEON/functions/NETranspose.h"

using namespace arm_compute;

/* ACL shapes are (width, height): a rows x cols matrix is TensorShape(cols, rows). */
static void init(Tensor &t, unsigned cols, unsigned rows) {
  t.allocator()->init(TensorInfo(TensorShape(cols, rows), 1, DataType::F32));
}
static void fill(Tensor &t, const std::vector<float> &v) {
  const unsigned cols = t.info()->dimension(0), rows = t.info()->dimension(1);
  for (unsigned y = 0; y < rows; ++y)
    for (unsigned x = 0; x < cols; ++x)
      *reinterpret_cast<float *>(t.ptr_to_element(Coordinates(x, y))) = v[y * cols + x];
}
static bool equals(Tensor &t, const std::vector<float> &v) {
  const unsigned cols = t.info()->dimension(0), rows = t.info()->dimension(1);
  if (v.size() != size_t(cols) * rows) return false;
  for (unsigned y = 0; y < rows; ++y)
    for (unsigned x = 0; x < cols; ++x) {
      float got = *reinterpret_cast<float *>(t.ptr_to_element(Coordinates(x, y)));
      if (std::fabs(got - v[y * cols + x]) > 1e-5f) { printf("  info (%u,%u) = %g, want %g\n", x, y, got, v[y * cols + x]); return false; }
    }
  return true;
}

int main() {
  QT_BEGIN("computelibrary");
  printf("  info %s, %u scheduler thread(s)\n", build_information().substr(0, 60).c_str(), Scheduler::get().num_threads());

  Tensor a, b, s;
  init(a, 8, 2); init(b, 8, 2); init(s, 8, 2);
  NEArithmeticAddition add;
  add.configure(&a, &b, &s, ConvertPolicy::SATURATE);
  a.allocator()->allocate(); b.allocator()->allocate(); s.allocator()->allocate();
  std::vector<float> va(16), vb(16), vs(16);
  for (int i = 0; i < 16; ++i) { va[i] = i * 0.5f; vb[i] = 100.f - i; vs[i] = va[i] + vb[i]; }
  fill(a, va); fill(b, vb);
  add.run();
  QT_CHECK(equals(s, vs), "NEArithmeticAddition, 16 elements");

  Tensor A, B, D;
  init(A, 3, 2); init(B, 2, 3); init(D, 2, 2);
  NEGEMM gemm;
  gemm.configure(&A, &B, nullptr, &D, 1.f, 0.f);
  A.allocator()->allocate(); B.allocator()->allocate(); D.allocator()->allocate();
  fill(A, {1, 2, 3, 4, 5, 6});
  fill(B, {7, 8, 9, 10, 11, 12});
  gemm.run();
  QT_CHECK(equals(D, {58, 64, 139, 154}), "NEGEMM: [[1,2,3],[4,5,6]] x [[7,8],[9,10],[11,12]]");

  Tensor x, y;
  init(x, 4, 2); init(y, 4, 2);
  NEActivationLayer relu;
  relu.configure(&x, &y, ActivationLayerInfo(ActivationLayerInfo::ActivationFunction::RELU));
  x.allocator()->allocate(); y.allocator()->allocate();
  fill(x, {-3, -0.5f, 0, 2, 7, -1, 4.5f, -8});
  relu.run();
  QT_CHECK(equals(y, {0, 0, 0, 2, 7, 0, 4.5f, 0}), "NEActivationLayer ReLU");

  Tensor pin, pout;
  pin.allocator()->init(TensorInfo(TensorShape(4U, 4U, 1U), 1, DataType::F32));
  pout.allocator()->init(TensorInfo(TensorShape(2U, 2U, 1U), 1, DataType::F32));
  NEPoolingLayer pool;
  pool.configure(&pin, &pout, PoolingLayerInfo(PoolingType::MAX, Size2D(2, 2), DataLayout::NCHW, PadStrideInfo(2, 2, 0, 0)));
  pin.allocator()->allocate(); pout.allocator()->allocate();
  fill(pin, {1, 5, 2, 0, 3, 4, 8, 1, 9, 0, 6, 6, 2, 7, 3, 2});
  pool.run();
  QT_CHECK(equals(pout, {5, 8, 9, 6}), "NEPoolingLayer 2x2 max, stride 2");

  Tensor t, tt;
  init(t, 3, 2); init(tt, 2, 3);
  NETranspose tr;
  tr.configure(&t, &tt);
  t.allocator()->allocate(); tt.allocator()->allocate();
  fill(t, {1, 2, 3, 4, 5, 6});
  tr.run();
  QT_CHECK(equals(tt, {1, 4, 2, 5, 3, 6}), "NETranspose 2x3 -> 3x2");
  QT_END();
}
