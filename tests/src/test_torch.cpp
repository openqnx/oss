// PyTorch (libtorch, C++): tensor creation, a matrix product through the CPU
// kernels, a reduction, and one backward pass through autograd.
#include "common.h"

#include <ATen/ATen.h>
#include <ATen/Parallel.h>
#include <torch/csrc/autograd/autograd.h>
#include <torch/csrc/autograd/generated/variable_factories.h>

int main() {
  QT_BEGIN("pytorch");

  at::Tensor a = at::arange(6, at::kFloat).reshape({2, 3});
  at::Tensor b = at::ones({3, 2}, at::kFloat);
  at::Tensor c = at::matmul(a, b);
  QT_CHECK(c.sizes() == at::IntArrayRef({2, 2}), "matmul shape 2x2");
  // rows of a are 0+1+2 and 3+4+5
  QT_CHECK(c[0][0].item<float>() == 3.0f && c[1][1].item<float>() == 12.0f, "matmul values");
  QT_CHECK(c.sum().item<float>() == 30.0f, "sum reduction");

  // d(sum(x*x))/dx = 2x
  at::Tensor x = torch::tensor({1.0f, 2.0f, 3.0f}, at::TensorOptions().requires_grad(true));
  at::Tensor y = (x * x).sum();
  torch::autograd::backward({y});
  at::Tensor g = x.grad();
  QT_CHECK(g.defined() && g[2].item<float>() == 6.0f, "autograd gradient 2x");

  printf("  info torch threads %d\n", at::get_num_threads());
  QT_END();
}
