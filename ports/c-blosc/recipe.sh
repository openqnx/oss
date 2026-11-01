# c-blosc 1.21.6: the Blosc blocking, shuffling meta-compressor (blosclz,
# lz4, zlib, zstd codecs behind one API), with qnx-ports' QNX change
# (patches/0001: its test suite's posix_memalign guard, which tests
# _POSIX_C_SOURCE, recognises QNX -- hazard 16; only the tests use it).
VERSION=1.21.6; GIT=https://github.com/Blosc/c-blosc.git; REF=v1.21.6
DEPS="lz4 zlib zstd"
# The codecs come from this tree's lz4, zlib and zstd rather than the copies
# bundled in internal-complibs/. Snappy stays off (upstream default: it would
# need a C++ snappy). SSE2/AVX2 shuffle kernels are chosen at run time on
# x86_64; aarch64 uses the generic ones.
CMAKE_ARGS_EXTRA=( -DBUILD_SHARED=ON -DBUILD_STATIC=ON -DBUILD_TESTS=OFF -DBUILD_FUZZERS=OFF
                   -DBUILD_BENCHMARKS=OFF -DPREFER_EXTERNAL_LZ4=ON -DPREFER_EXTERNAL_ZLIB=ON
                   -DPREFER_EXTERNAL_ZSTD=ON -DDEACTIVATE_SNAPPY=ON -DCMAKE_POSITION_INDEPENDENT_CODE=ON )
