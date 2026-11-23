# ccache, the compiler cache: it hashes a compilation's preprocessed input
# and options and serves the object file from its cache on a repeat.
# Version as in QNX's own package list.
VERSION=4.14.3
TARBALL=https://github.com/ccache/ccache/releases/download/v4.14.3/ccache-4.14.3.tar.xz
SHA256=e1999132cf7dbce48de7d946b767f3e26ea09adcb5daa4a7103a8fa09ee0637d
# zstd: ccache compresses its cache entries with it; xxhash: its fast
# checksums (ccache bundles no copy of it). fmt, tl-expected, blake3 and
# nonstd-span are the copies bundled in src/third_party (DEP_*=BUNDLED: not
# whatever staging happens to have); DEPS=LOCAL: nothing is downloaded.
DEPS="zstd xxhash"
BUILDSYS=cmake
# The toolchain file's "-isystem <staging>/include" on every compile is
# dropped: it came ahead of the bundled copies' include directories (which
# ccache marks SYSTEM as well), so the staging tree's fmt 11 and tl/expected
# headers were used instead -- fmt 12's format.cc then failed with "'locale_ref'
# has not been declared". zstd and xxhash still bring the staging include
# directory in, behind the bundled ones, through their imported targets.
# No remote storage back ends: Redis would need hiredis (not ported), and
# HTTP storage is the only use of the bundled cpp-httplib (OpenSSL-free
# plain HTTP). No documentation (asciidoctor), no unit tests (doctest).
CMAKE_ARGS_EXTRA=( -DDEPS=LOCAL -DDEP_FMT=BUNDLED -DDEP_TLEXPECTED=BUNDLED -DREDIS_STORAGE_BACKEND=OFF -DHTTP_STORAGE_BACKEND=OFF
                   -DENABLE_TESTING=OFF -DENABLE_DOCUMENTATION=OFF -DUSE_FASTER_LINKER=OFF
                   -DSTATIC_LINK=OFF
                   -DCMAKE_C_FLAGS= -DCMAKE_CXX_FLAGS=-std=gnu++17 )
