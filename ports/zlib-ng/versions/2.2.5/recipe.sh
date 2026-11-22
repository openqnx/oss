# zlib-ng (zlib with SIMD optimisations), its native API (libz-ng) next to
# the zlib component. Version as in the QNX Everywhere 8.0.4r1 image.
VERSION=2.2.5; GIT=https://github.com/zlib-ng/zlib-ng.git; REF=2.2.5
CMAKE_ARGS_EXTRA=( -DZLIB_COMPAT=OFF -DZLIB_ENABLE_TESTS=OFF -DZLIBNG_ENABLE_TESTS=OFF -DWITH_GTEST=OFF -DBUILD_SHARED_LIBS=ON )
