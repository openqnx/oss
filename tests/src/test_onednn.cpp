#include "common.h"
#include <oneapi/dnnl/dnnl.hpp>
#include <cmath>
#include <vector>
/* oneDNN: a matrix product (16x24 by 24x20, bias and ReLU as post-op), a
 * 3x3 convolution (1x4x9x9 -> 1x8x7x7, bias), and max pooling run through the
 * CPU engine and compared with naive loops here (tolerance 1e-3; inputs are
 * multiples of 1/16, so the products are exact in float). Also reports which
 * instruction set oneDNN's JIT chose and checks the version is 3.14.1. */
using namespace dnnl;
static float val(int i, int m) { return (float)((i * 37 + m) % 17 - 8) / 16.0f; }
static bool same(const std::vector<float> &a, const std::vector<float> &b, const char *what) {
  for (size_t i = 0; i < a.size(); i++)
    if (std::fabs(a[i] - b[i]) > 1e-3f) { printf("  info %s[%zu] = %f, expected %f\n", what, i, a[i], b[i]); return false; }
  return true;
}
int main() {
  QT_BEGIN("oneDNN");
  const dnnl_version_t *v = dnnl_version();
  printf("  info oneDNN %d.%d.%d (%s), cpu isa: %d\n", v->major, v->minor, v->patch, v->hash, (int)get_effective_cpu_isa());
  QT_CHECK(v->major == 3 && v->minor == 14 && v->patch == 1, "version 3.14.1");
  engine eng(engine::kind::cpu, 0);
  stream s(eng);
  using tag = memory::format_tag; using dt = memory::data_type;

  { /* matmul + bias + relu */
    const int M = 16, K = 24, N = 20;
    std::vector<float> A(M * K), B(K * N), bias(N), C(M * N), ref(M * N);
    for (int i = 0; i < M * K; i++) A[i] = val(i, 1);
    for (int i = 0; i < K * N; i++) B[i] = val(i, 2);
    for (int i = 0; i < N; i++) bias[i] = val(i, 3);
    for (int m = 0; m < M; m++) for (int n = 0; n < N; n++) {
      float acc = bias[n]; for (int k = 0; k < K; k++) acc += A[m * K + k] * B[k * N + n];
      ref[m * N + n] = acc > 0 ? acc : 0; }
    memory::desc ad({M, K}, dt::f32, tag::ab), bd({K, N}, dt::f32, tag::ab), cd({M, N}, dt::f32, tag::ab), biasd({1, N}, dt::f32, tag::ab);
    post_ops po; po.append_eltwise(algorithm::eltwise_relu, 0.f, 0.f);
    primitive_attr attr; attr.set_post_ops(po);
    matmul::primitive_desc pd(eng, ad, bd, biasd, cd, attr);
    printf("  info matmul: %s\n", pd.impl_info_str());
    memory am(ad, eng, A.data()), bm(bd, eng, B.data()), cm(cd, eng, C.data()), biasm(biasd, eng, bias.data());
    matmul(pd).execute(s, {{DNNL_ARG_SRC, am}, {DNNL_ARG_WEIGHTS, bm}, {DNNL_ARG_BIAS, biasm}, {DNNL_ARG_DST, cm}});
    s.wait();
    QT_CHECK(same(C, ref, "matmul"), "matmul 16x24 * 24x20 + bias, ReLU = reference");
  }
  { /* convolution, then max pooling 2x2 stride 1 */
    const int IC = 4, OC = 8, H = 9, W = 9, KH = 3, OH = H - KH + 1, PH = OH - 1;
    std::vector<float> src(IC * H * W), wei(OC * IC * KH * KH), bias(OC), dst(OC * OH * OH), ref(OC * OH * OH);
    for (size_t i = 0; i < src.size(); i++) src[i] = val((int)i, 4);
    for (size_t i = 0; i < wei.size(); i++) wei[i] = val((int)i, 5);
    for (int i = 0; i < OC; i++) bias[i] = val(i, 6);
    for (int o = 0; o < OC; o++) for (int y = 0; y < OH; y++) for (int x = 0; x < OH; x++) {
      float acc = bias[o];
      for (int c = 0; c < IC; c++) for (int ky = 0; ky < KH; ky++) for (int kx = 0; kx < KH; kx++)
        acc += wei[((o * IC + c) * KH + ky) * KH + kx] * src[(c * H + y + ky) * W + x + kx];
      ref[(o * OH + y) * OH + x] = acc; }
    memory::desc sd({1, IC, H, W}, dt::f32, tag::nchw), wd({OC, IC, KH, KH}, dt::f32, tag::oihw),
                 bd({OC}, dt::f32, tag::x), dd({1, OC, OH, OH}, dt::f32, tag::nchw);
    convolution_forward::primitive_desc pd(eng, prop_kind::forward_inference, algorithm::convolution_direct,
                                           sd, wd, bd, dd, {1, 1}, {0, 0}, {0, 0});
    printf("  info convolution: %s\n", pd.impl_info_str());
    memory sm(sd, eng, src.data()), wm(wd, eng, wei.data()), bm(bd, eng, bias.data()), dm(dd, eng, dst.data());
    convolution_forward(pd).execute(s, {{DNNL_ARG_SRC, sm}, {DNNL_ARG_WEIGHTS, wm}, {DNNL_ARG_BIAS, bm}, {DNNL_ARG_DST, dm}});
    s.wait();
    QT_CHECK(same(dst, ref, "conv"), "convolution 1x4x9x9 (*) 8x4x3x3 + bias = reference");

    std::vector<float> pool(OC * PH * PH), pref(OC * PH * PH);
    for (int o = 0; o < OC; o++) for (int y = 0; y < PH; y++) for (int x = 0; x < PH; x++) {
      float mx = -1e30f;
      for (int ky = 0; ky < 2; ky++) for (int kx = 0; kx < 2; kx++) mx = std::max(mx, ref[(o * OH + y + ky) * OH + x + kx]);
      pref[(o * PH + y) * PH + x] = mx; }
    memory::desc pdd({1, OC, PH, PH}, dt::f32, tag::nchw);
    pooling_forward::primitive_desc ppd(eng, prop_kind::forward_inference, algorithm::pooling_max, dd, pdd,
                                        {1, 1}, {2, 2}, {0, 0}, {0, 0}, {0, 0});
    memory pm(pdd, eng, pool.data());
    pooling_forward(ppd).execute(s, {{DNNL_ARG_SRC, dm}, {DNNL_ARG_DST, pm}});
    s.wait();
    QT_CHECK(same(pool, pref, "pool"), "max pooling 2x2 = reference");
  }
  QT_END();
}
