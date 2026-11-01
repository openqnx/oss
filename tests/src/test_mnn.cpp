#include "common.h"
#include <MNN/Interpreter.hpp>
#include <MNN/Tensor.hpp>
#include <MNN/expr/Expr.hpp>
#include <MNN/expr/ExprCreator.hpp>
#include <MNN/expr/Module.hpp>
#include <cmath>
#include <memory>
#include <vector>
using namespace MNN::Express;
/* MNN: a small network built in code with the express API -- a 3x3
 * convolution (2 -> 3 channels, bias, ReLU) on a 1x2x6x6 input, global sum
 * over the image, a 3x4 matrix product and softmax -- run on the CPU and
 * compared, element by element, with the same arithmetic done in plain C++
 * here. Then the graph is serialized to MNN's model format, loaded back as a
 * Module and through the Interpreter/Session API, run on a second input, and
 * compared again. All values are small integers/8ths, so float rounding
 * cannot hide a wrong result (tolerance 1e-3). */
enum { IC = 2, OC = 3, H = 6, W = 6, K = 3, OH = H - K + 1, OW = W - K + 1, NO = 4 };
static std::vector<float> wconv() { std::vector<float> w(OC * IC * K * K);
  for (size_t i = 0; i < w.size(); i++) w[i] = (float)((int)(i * 7 % 11) - 5) / 8.0f; return w; }
static std::vector<float> bconv() { return { 0.5f, -1.0f, 0.25f }; }
static std::vector<float> wfc() { std::vector<float> w(OC * NO);
  for (size_t i = 0; i < w.size(); i++) w[i] = (float)((int)(i * 5 % 7) - 3) / 64.0f; return w; }
static void input(float *x, int seed) { for (int i = 0; i < IC * H * W; i++) x[i] = (float)((i * 3 + seed) % 9 - 4) / 4.0f; }
/* reference: conv (OIHW weights, NCHW data), relu, sum over HxW, x wfc, softmax */
static std::vector<float> reference(const float *x) {
  auto w = wconv(), b = bconv(), f = wfc();
  float s[OC] = {0};
  for (int o = 0; o < OC; o++)
    for (int y = 0; y < OH; y++)
      for (int xx = 0; xx < OW; xx++) {
        float acc = b[o];
        for (int c = 0; c < IC; c++)
          for (int ky = 0; ky < K; ky++)
            for (int kx = 0; kx < K; kx++)
              acc += w[((o * IC + c) * K + ky) * K + kx] * x[(c * H + y + ky) * W + xx + kx];
        s[o] += acc > 0 ? acc : 0;
      }
  std::vector<float> z(NO), p(NO); float mx = -1e30f, sum = 0;
  for (int j = 0; j < NO; j++) { z[j] = 0; for (int o = 0; o < OC; o++) z[j] += s[o] * f[o * NO + j]; mx = std::max(mx, z[j]); }
  for (int j = 0; j < NO; j++) { p[j] = std::exp(z[j] - mx); sum += p[j]; }
  for (int j = 0; j < NO; j++) p[j] /= sum;
  return p;
}
static bool close(const float *a, const std::vector<float> &b) {
  for (size_t i = 0; i < b.size(); i++) if (std::fabs(a[i] - b[i]) > 1e-3f) {
    printf("  info [%zu] %f, expected %f\n", i, a[i], b[i]); return false; }
  return true;
}
int main() {
  QT_BEGIN("MNN");
  auto x = _Input({1, IC, H, W}, NCHW, halide_type_of<float>());
  x->setName("x");
  auto conv = _Conv(wconv(), bconv(), _Convert(x, NC4HW4), {IC, OC}, {K, K}, VALID, {1, 1}, {1, 1}, 1, {0, 0}, true);
  auto pooled = _ReduceSum(_Convert(conv, NCHW), {2, 3});                 /* 1 x OC */
  auto fw = wfc();
  auto logits = _MatMul(pooled, _Const(fw.data(), {OC, NO}, NCHW));      /* 1 x NO */
  auto prob = _Softmax(logits, -1);
  prob->setName("prob");

  input(x->writeMap<float>(), 0);
  float xin[IC * H * W]; input(xin, 0);
  auto want = reference(xin);
  printf("  info expected %.4f %.4f %.4f %.4f\n", want[0], want[1], want[2], want[3]);
  const float *got = prob->readMap<float>();
  QT_CHECK(got != nullptr && prob->getInfo()->size == NO, "graph computed: 4 outputs");
  if (got) printf("  info got      %.4f %.4f %.4f %.4f\n", got[0], got[1], got[2], got[3]);
  QT_CHECK(got && close(got, want), "conv + relu + sum + matmul + softmax match the reference");
  float tot = 0; if (got) for (int i = 0; i < NO; i++) tot += got[i];
  QT_CHECK(std::fabs(tot - 1.0f) < 1e-4f, "softmax sums to 1");

  /* serialize, reload, run on another input */
  std::vector<int8_t> buf = Variable::save({prob});
  QT_CHECK(buf.size() > 100, "graph serialized to an MNN model");
  float x2[IC * H * W]; input(x2, 5);
  auto want2 = reference(x2);
  std::unique_ptr<MNN::Express::Module> mod(MNN::Express::Module::load({"x"}, {"prob"}, (const uint8_t *)buf.data(), buf.size()));
  QT_CHECK(mod != nullptr, "model loaded as a Module");
  if (mod) {
    auto in = _Input({1, IC, H, W}, NCHW, halide_type_of<float>());
    memcpy(in->writeMap<float>(), x2, sizeof x2);
    auto outs = mod->onForward({in});
    const float *r = outs.size() == 1 ? outs[0]->readMap<float>() : nullptr;
    QT_CHECK(r && close(r, want2), "Module::onForward on a second input matches the reference");
  }
  std::unique_ptr<MNN::Interpreter> net(MNN::Interpreter::createFromBuffer(buf.data(), buf.size()));
  QT_CHECK(net != nullptr, "model loaded by the Interpreter");
  if (net) {
    MNN::ScheduleConfig sc; sc.type = MNN_FORWARD_CPU; sc.numThread = 2;
    auto session = net->createSession(sc);
    auto tin = net->getSessionInput(session, "x");
    std::unique_ptr<MNN::Tensor> host(MNN::Tensor::create<float>({1, IC, H, W}, x2, MNN::Tensor::CAFFE));
    tin->copyFromHostTensor(host.get());
    QT_CHECK(net->runSession(session) == MNN::NO_ERROR, "session ran (2 threads)");
    auto tout = net->getSessionOutput(session, "prob");
    MNN::Tensor hout(tout, MNN::Tensor::CAFFE);
    tout->copyToHostTensor(&hout);
    QT_CHECK(close(hout.host<float>(), want2), "Interpreter session output matches the reference");
  }
  QT_END();
}
