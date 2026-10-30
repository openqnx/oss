# ncnn: Tencent's neural-network inference framework, tuned for CPUs (x86 SSE/AVX,
# Arm NEON), as a shared library and benchncnn. Threads through its own pool:
# QNX's GCC has no OpenMP runtime. No Vulkan (not in the SDP).
VERSION=20260526; GIT=https://github.com/Tencent/ncnn.git; REF=20260526
DEPS=""
BUILDSYS=cmake
CMAKE_ARGS_EXTRA=( -DNCNN_SHARED_LIB=ON -DNCNN_VULKAN=OFF -DNCNN_OPENMP=OFF -DNCNN_BUILD_TOOLS=OFF -DNCNN_BUILD_EXAMPLES=OFF -DNCNN_BUILD_BENCHMARK=ON -DNCNN_BUILD_TESTS=OFF -DNCNN_PYTHON=OFF -DNCNN_SIMPLEOCV=ON )
