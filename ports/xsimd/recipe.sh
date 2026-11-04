# xsimd: header-only C++ wrappers for SIMD intrinsics (SSE/AVX on x86_64,
# NEON on aarch64) with vectorised math functions; xtensor's SIMD backend.
# Header-only: installs include/xsimd, xsimd.pc and the CMake package.
VERSION=14.3.0; GIT=https://github.com/xtensor-stack/xsimd.git; REF=14.3.0
BUILDSYS=cmake
# Tests, benchmarks and examples are not installed (and the tests fetch
# doctest): off.
CMAKE_ARGS_EXTRA=( -DBUILD_TESTS=OFF -DBUILD_BENCHMARK=OFF -DBUILD_EXAMPLES=OFF )
