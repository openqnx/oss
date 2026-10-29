#include "common.h"
#include <onnxruntime_cxx_api.h>
/* Builds an ORT environment, session options and tensors -- everything except
 * loading a model file, which would need an .onnx asset on the target. */
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
  } catch (const Ort::Exception &e) {
    printf("  FAIL Ort::Exception: %s\n", e.what());
    return 1;
  }
  QT_END();
}
