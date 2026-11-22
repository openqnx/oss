VERSION=53.3.2; GIT=https://github.com/ARM-software/ComputeLibrary.git; REF=v53.3.2
# Arm Compute Library: NEON/SVE kernels for Arm CPUs -- aarch64 only.
ARCHES_SUPPORTED="aarch64le"
CMAKE_ARGS_EXTRA=( -DARM_COMPUTE_BUILD_SHARED_LIB=ON -DARM_COMPUTE_ENABLE_OPENMP=OFF
                   -DARM_COMPUTE_BUILD_EXAMPLES=OFF -DARM_COMPUTE_BUILD_TESTING=OFF
                   "-DCMAKE_CXX_FLAGS=-std=gnu++17 -isystem $STAGE$PREFIX/include -D_QNX_SOURCE=1" )
# The library bundles a copy of the Khronos OpenCL headers and installs it as
# include/CL. The C headers are the opencl-headers component's (the same
# files, newer): only the C++ binding CL/opencl.hpp, which
# arm_compute/core/CL/OpenCL.h includes, stays here.
DEPS="opencl-headers"
do_install() {
  DESTDIR="$STAGE" cmake --install "$B" || return 1
  find "$STAGE$PREFIX/include/CL" -type f ! -name opencl.hpp -delete
}
