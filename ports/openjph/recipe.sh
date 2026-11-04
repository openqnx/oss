# openjph: HTJ2K (High-Throughput JPEG 2000, ITU-T T.814) encoder and decoder
# library, with ojph_compress/ojph_expand. Added as a dependency of OpenEXR 3.4,
# whose HTJ2K compression is built on it (otherwise OpenEXR downloads a copy at
# configure time). 0.24.1 is the release OpenEXR 3.4.1 pins.
VERSION=0.24.1; GIT=https://github.com/aous72/OpenJPH.git; REF=0.24.1
DEPS="libtiff"
BUILDSYS=cmake
# Shared library and the two tools, with TIFF input/output (libtiff is here).
# The SIMD paths are chosen at run time from the CPU's features.
# C++17: QNX declares aligned_alloc() only for C11/C++17 and later (the
# project defaults to C++14, where it is "not declared in this scope").
CMAKE_ARGS_EXTRA=( -DCMAKE_CXX_STANDARD=17 -DBUILD_SHARED_LIBS=ON -DOJPH_BUILD_TESTS=OFF -DOJPH_BUILD_EXECUTABLES=ON
                   -DOJPH_ENABLE_TIFF_SUPPORT=ON )
