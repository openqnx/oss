#include "common.h"
#include <onnxruntime_cxx_api.h>
/* Builds an ORT environment, session options and tensors, then loads a small
 * MatMul model from memory and checks its products. */
#include <cmath>
#include <cstring>
#include <vector>
/* A model with one node, Y = MatMul(A, B), float (opset 13, ir 8). The same
 * bytes with element type 11 in the three value infos are the double model. */
static const unsigned char MATMUL_ONNX[] = {
  0x08, 0x08, 0x3a, 0x5c, 0x0a, 0x11, 0x0a, 0x01, 0x41, 0x0a, 0x01, 0x42,
  0x12, 0x01, 0x59, 0x22, 0x06, 0x4d, 0x61, 0x74, 0x4d, 0x75, 0x6c, 0x12,
  0x02, 0x6d, 0x6d, 0x5a, 0x15, 0x0a, 0x01, 0x41, 0x12, 0x10, 0x0a, 0x0e,
  0x08, 0x01, 0x12, 0x0a, 0x0a, 0x03, 0x12, 0x01, 0x6d, 0x0a, 0x03, 0x12,
  0x01, 0x6b, 0x5a, 0x15, 0x0a, 0x01, 0x42, 0x12, 0x10, 0x0a, 0x0e, 0x08,
  0x01, 0x12, 0x0a, 0x0a, 0x03, 0x12, 0x01, 0x6b, 0x0a, 0x03, 0x12, 0x01,
  0x6e, 0x62, 0x15, 0x0a, 0x01, 0x59, 0x12, 0x10, 0x0a, 0x0e, 0x08, 0x01,
  0x12, 0x0a, 0x0a, 0x03, 0x12, 0x01, 0x6d, 0x0a, 0x03, 0x12, 0x01, 0x6e,
  0x42, 0x04, 0x0a, 0x00, 0x10, 0x0d
};
/* Products through the SGEMM/DGEMM kernels against a plain triple loop. The
 * x86_64 assembly kernels were once assembled with wrong vector widths on QNX
 * (see ports/onnxruntime/patches/0002): products with 16 or more columns
 * came back wrong, smaller ones were right. */
template <typename T> static int matmul_wrong(Ort::Env &env, int threads) {
  std::vector<unsigned char> model(MATMUL_ONNX, MATMUL_ONNX + sizeof MATMUL_ONNX);
  if (sizeof(T) == 8)
    for (size_t i = 0; i + 3 < model.size(); i++)
      if (model[i] == 0x0a && model[i + 1] == 0x0e && model[i + 2] == 0x08 && model[i + 3] == 0x01) model[i + 3] = 0x0b;
  Ort::SessionOptions so; so.SetIntraOpNumThreads(threads);
  Ort::Session s(env, model.data(), model.size(), so);
  auto mi = Ort::MemoryInfo::CreateCpu(OrtDeviceAllocator, OrtMemTypeDefault);
  static const int sizes[][3] = {{1,1,1},{2,3,4},{8,8,8},{3,16,5},{16,16,16},{17,33,9},{32,32,32},{64,64,31},{192,192,31},{192,192,1},{1,192,31},{192,1,31},{100,300,50}};
  int wrong = 0;
  for (auto &z : sizes) {
    int M = z[0], K = z[1], N = z[2];
    std::vector<T> A(M * K), B(K * N), R(M * N, 0);
    for (int i = 0; i < M * K; i++) A[i] = ((i * 37 % 101) - 50) / (T)50;
    for (int i = 0; i < K * N; i++) B[i] = ((i * 53 % 97) - 48) / (T)48;
    for (int i = 0; i < M; i++) for (int k = 0; k < K; k++) for (int j = 0; j < N; j++) R[i * N + j] += A[i * K + k] * B[k * N + j];
    int64_t sa[] = {M, K}, sb[] = {K, N};
    std::vector<Ort::Value> in;
    in.push_back(Ort::Value::CreateTensor<T>(mi, A.data(), A.size(), sa, 2));
    in.push_back(Ort::Value::CreateTensor<T>(mi, B.data(), B.size(), sb, 2));
    const char *inn[] = {"A", "B"}; const char *outn[] = {"Y"};
    auto out = s.Run(Ort::RunOptions{nullptr}, inn, in.data(), 2, outn, 1);
    auto shp = out[0].GetTensorTypeAndShapeInfo().GetShape();
    const T *y = out[0].GetTensorData<T>(); int bad = (shp.size() != 2 || shp[0] != M || shp[1] != N);
    for (int i = 0; !bad && i < M * N; i++) if (!(std::fabs((double)(y[i] - R[i])) <= 1e-3)) bad = 1;
    if (bad) { printf("  info %s %dx%d * %dx%d, %d thread(s): wrong\n", sizeof(T) == 8 ? "double" : "float", M, K, K, N, threads); wrong++; }
  }
  return wrong;
}
int main() {
  QT_BEGIN("onnxruntime");
  printf("  info onnxruntime %s\n", Ort::GetVersionString().c_str());
  try {
    Ort::Env env(ORT_LOGGING_LEVEL_WARNING, "qnx_port_test");
    QT_CHECK(env != nullptr, "create Env");
    Ort::SessionOptions so;
    so.SetIntraOpNumThreads(2);
    so.SetGraphOptimizationLevel(GraphOptimizationLevel::ORT_ENABLE_BASIC);
    QT_CHECK(true, "SessionOptions configured");
    auto mem = Ort::MemoryInfo::CreateCpu(OrtArenaAllocator, OrtMemTypeDefault);
    QT_CHECK(mem != nullptr, "CreateCpu MemoryInfo");
    std::vector<float> vals{1.f, 2.f, 3.f, 4.f, 5.f, 6.f};
    std::vector<int64_t> shape{2, 3};
    auto t = Ort::Value::CreateTensor<float>(mem, vals.data(), vals.size(),
                                             shape.data(), shape.size());
    QT_CHECK(t.IsTensor(), "CreateTensor yields a tensor");
    auto info = t.GetTensorTypeAndShapeInfo();
    QT_CHECK(info.GetElementCount() == 6, "tensor element count");
    QT_CHECK(info.GetShape() == shape, "tensor shape round trip");
    QT_CHECK(t.GetTensorData<float>()[5] == 6.f, "tensor data readback");
    auto providers = Ort::GetAvailableProviders();
    QT_CHECK(!providers.empty(), "at least one execution provider");
    for (auto &p : providers) printf("  info provider %s\n", p.c_str());
    QT_CHECK(matmul_wrong<float>(env, 1) == 0, "MatMul model from memory: 13 float products, 1x1 to 192x192 and 100x300x50, equal a plain loop");
    QT_CHECK(matmul_wrong<double>(env, 1) == 0, "the same products in double");
    QT_CHECK(matmul_wrong<float>(env, 4) == 0 && matmul_wrong<double>(env, 4) == 0, "and with 4 intra-op threads");
  } catch (const Ort::Exception &e) {
    printf("  FAIL Ort::Exception: %s\n", e.what());
    return 1;
  }
  QT_END();
}
