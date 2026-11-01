# highwayhash: Google's fast keyed hash functions -- HighwayHash (SIMD:
# AVX2/SSE4.1 on x86_64, NEON on aarch64, chosen at run time) and SipHash --
# as libhighwayhash. Built for riegeli, which hashes its chunks with it.
# Upstream has no releases: the newest commit (2024-04-18).
VERSION=0_git20240418; GIT=https://github.com/google/highwayhash.git; COMMIT=f8381f3331d9c56a9792f9b4a35f61c41108c39e
DEPS=""
BUILDSYS=cmake
# patches/0001: the CMake build makes the library usable from outside --
# with the run-time target dispatch (instruction_sets.cc), a soname and
# install rules -- and builds its tests and benchmarks only on request
# (their timer code has no QNX port). patches/0002: -fno-strict-aliasing, without
# which GCC -O3 miscomputes the portable target (not QNX-specific).
CMAKE_ARGS_EXTRA=( -DBUILD_SHARED_LIBS=ON -DHH_BUILD_TESTS=OFF )
