# xtensor: header-only C++ library for multi-dimensional arrays with
# NumPy-style broadcasting and lazy expression evaluation. Header-only:
# installs include/xtensor, xtensor.pc and the CMake package.
VERSION=0.27.1; GIT=https://github.com/xtensor-stack/xtensor.git; REF=0.27.1
# xtl is required; xsimd turns on SIMD evaluation of expressions
# (XTENSOR_USE_XSIMD): the installed xtensorConfig.cmake then finds xsimd
# and defines XTENSOR_USE_XSIMD for consumers, as distribution builds do.
DEPS="xtl xsimd"
# Header-only: name the headers a consumer needs with it.
RUNTIME_DEPS="xtl-dev xsimd-dev"
BUILDSYS=cmake
# TBB and OpenMP parallelisation stay off (the upstream default). Tests and
# benchmarks are not installed: off.
CMAKE_ARGS_EXTRA=( -DXTENSOR_USE_XSIMD=ON -DXTENSOR_USE_TBB=OFF -DXTENSOR_USE_OPENMP=OFF
                   -DBUILD_TESTS=OFF -DBUILD_BENCHMARK=OFF )
