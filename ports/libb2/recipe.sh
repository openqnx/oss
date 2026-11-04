# libb2: the BLAKE2 reference library -- the BLAKE2b, BLAKE2s, BLAKE2bp and
# BLAKE2sp hash functions (keyed and unkeyed), from the BLAKE2 authors.
VERSION=0.98.1
TARBALL=https://github.com/BLAKE2/libb2/releases/download/v0.98.1/libb2-0.98.1.tar.gz
SHA256=53626fddce753c454a3fea581cbbc7fe9bbcf0bc70416d48fdbbf5d87ef6c72e
BUILDSYS=autotools
# Never -march=native (that would be the build machine's CPU). On x86_64 a
# "fat" library: the SSE2/SSSE3/SSE4.1/AVX/XOP variants are all built and
# the fastest one the CPU supports is picked at run time (cpuid). aarch64
# has the portable C code only. OpenMP (for the parallel bp/sp variants) is
# not in the SDP; they run sequentially.
CONF_ARGS_EXTRA=( --disable-static --disable-native --disable-openmp
                  $([ "$QNX_ARCH" = x86_64 ] && echo --enable-fat || echo --disable-fat) )
