// TensorFlow Lite: build a one-node graph (out = a + b) through the C++
// interpreter API, with the ADD kernel taken from the builtin op resolver,
// then allocate, run and read it back. No model file needed, and it exercises
// the resolver, the arena allocator and a real kernel.
#include "common.h"

#include <tensorflow/lite/builtin_ops.h>
#include <tensorflow/lite/core/c/builtin_op_data.h>
#include <tensorflow/lite/interpreter.h>
#include <tensorflow/lite/kernels/register.h>

#include <cstdlib>

int main() {
  QT_BEGIN("tensorflow-lite");

  tflite::ops::builtin::BuiltinOpResolver resolver;
  const TfLiteRegistration* add = resolver.FindOp(tflite::BuiltinOperator_ADD, 1);
  QT_CHECK(add != nullptr, "builtin ADD kernel registered");
  if (!add) QT_END();

  tflite::Interpreter interp;
  QT_CHECK(interp.AddTensors(3) == kTfLiteOk, "add 3 tensors");
  QT_CHECK(interp.SetInputs({0, 1}) == kTfLiteOk, "set inputs");
  QT_CHECK(interp.SetOutputs({2}) == kTfLiteOk, "set outputs");
  TfLiteQuantization q{};
  for (int i = 0; i < 3; i++)
    QT_CHECK(interp.SetTensorParametersReadWrite(i, kTfLiteFloat32, "t", {4}, q) == kTfLiteOk,
          "tensor parameters");

  // The interpreter takes ownership of builtin_data and frees it with free().
  auto* params = static_cast<TfLiteAddParams*>(std::malloc(sizeof(TfLiteAddParams)));
  params->activation = kTfLiteActNone;
  params->pot_scale_int16 = false;
  QT_CHECK(interp.AddNodeWithParameters({0, 1}, {2}, nullptr, 0, params, add) == kTfLiteOk,
        "add ADD node");
  QT_CHECK(interp.AllocateTensors() == kTfLiteOk, "allocate tensors");

  float* a = interp.typed_tensor<float>(0);
  float* b = interp.typed_tensor<float>(1);
  for (int i = 0; i < 4; i++) { a[i] = 1.5f * i; b[i] = 10.0f; }
  QT_CHECK(interp.Invoke() == kTfLiteOk, "invoke");

  const float* out = interp.typed_tensor<float>(2);
  bool ok = true;
  for (int i = 0; i < 4; i++) ok = ok && out[i] == 1.5f * i + 10.0f;
  QT_CHECK(ok, "a + b computed");
  QT_END();
}
