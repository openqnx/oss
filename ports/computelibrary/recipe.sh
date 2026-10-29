VERSION=53.0.0; GIT=https://github.com/ARM-software/ComputeLibrary.git; REF=v53.0.0
# Arm Compute Library: NEON/SVE kernels for Arm CPUs -- aarch64 only.
ARCHES_SUPPORTED="aarch64le"
CMAKE_ARGS_EXTRA=( -DARM_COMPUTE_BUILD_SHARED_LIB=ON -DARM_COMPUTE_ENABLE_OPENMP=OFF
                   -DARM_COMPUTE_BUILD_EXAMPLES=OFF -DARM_COMPUTE_BUILD_TESTING=OFF
                   "-DCMAKE_CXX_FLAGS=-std=gnu++17 -isystem $STAGE$PREFIX/include -D_QNX_SOURCE=1" )
