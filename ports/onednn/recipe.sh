# oneDNN: the UXL Foundation's (formerly Intel's) deep-learning primitives
# library -- convolution, matmul, pooling, normalization and the rest, JIT
# compiled for the CPU (Xbyak on x86_64, Xbyak_aarch64 on Arm) -- with QNX's
# change (patches/0001, from github.com/qnx-ports/oneDNN, branch
# qnx-openvino-2026.1.2) and a QNX CPU-information backend for Xbyak_aarch64
# (patches/0002: features, cores and caches from the system page; without it
# the Arm JIT does not build for QNX).
VERSION=3.14.1; GIT=https://github.com/uxlfoundation/oneDNN.git; REF=v3.14.1
DEPS=""
BUILDSYS=cmake
# Sequential CPU runtime: QNX's GCC has no OpenMP runtime and there is no TBB
# here. No GPU runtime (no OpenCL/SYCL in the SDP), no Arm Compute Library.
CMAKE_ARGS_EXTRA=( -DONEDNN_CPU_RUNTIME=SEQ -DONEDNN_GPU_RUNTIME=NONE -DONEDNN_BUILD_EXAMPLES=OFF -DONEDNN_BUILD_TESTS=OFF
                   -DONEDNN_BUILD_DOC=OFF -DONEDNN_BUILD_GRAPH=ON -DONEDNN_LIBRARY_TYPE=SHARED -DONEDNN_WERROR=OFF )
