# MNN: Alibaba's lightweight deep-learning inference engine (CPU), as the
# shared libraries libMNN and libMNN_Express (the express API that builds and
# runs graphs in code, and the Module API). QNX's changes
# (github.com/qnx-ports/MNN, "add compatibility support for QNX Neutrino") are
# upstream since 3.1.2.
VERSION=3.6.2; GIT=https://github.com/alibaba/MNN.git; REF=3.6.2
DEPS=""
BUILDSYS=cmake
# Threads through MNN's own pool: QNX's GCC has no OpenMP runtime. No GPU
# backends (no Vulkan or OpenCL in the SDP), no model converter (it would
# need its own protobuf), no tools (MNN does not install them). -fPIC: the
# library is linked from CMake object libraries, which on x86_64 are not PIC
# by themselves.
CMAKE_ARGS_EXTRA=( -DMNN_BUILD_SHARED_LIBS=ON -DCMAKE_POSITION_INDEPENDENT_CODE=ON -DMNN_SEP_BUILD=ON -DMNN_USE_THREAD_POOL=ON -DMNN_OPENMP=OFF
                   -DMNN_VULKAN=OFF -DMNN_OPENCL=OFF -DMNN_OPENGL=OFF -DMNN_BUILD_CONVERTER=OFF
                   -DMNN_BUILD_TOOLS=OFF -DMNN_BUILD_DEMO=OFF -DMNN_BUILD_TEST=OFF -DMNN_BUILD_BENCHMARK=OFF )
