# ccache, the compiler cache: it hashes a compilation's preprocessed input
# and options and serves the object file from its cache on a repeat.
# Version as in QNX's own package list.
VERSION=4.12.1
TARBALL=https://github.com/ccache/ccache/releases/download/v4.12.1/ccache-4.12.1.tar.xz
SHA256=42e6c4b260cffd92dfa755e26390f45f5842e8c1a5f64f2b2e68adfbe82d1542
# zstd: ccache compresses its cache entries with it; xxhash: its fast
# checksums (ccache bundles no copy of it). fmt, tl-expected, blake3 and
# nonstd-span are the copies bundled in src/third_party (DEP_*=BUNDLED: not
# whatever staging happens to have); DEPS=LOCAL: nothing is downloaded.
DEPS="zstd xxhash"
BUILDSYS=cmake
# No remote storage back ends: Redis would need hiredis (not ported), and
# HTTP storage is the only use of the bundled cpp-httplib (OpenSSL-free
# plain HTTP). No documentation (asciidoctor), no unit tests (doctest).
CMAKE_ARGS_EXTRA=( -DDEPS=LOCAL -DDEP_FMT=BUNDLED -DDEP_TLEXPECTED=BUNDLED -DREDIS_STORAGE_BACKEND=OFF -DHTTP_STORAGE_BACKEND=OFF
                   -DENABLE_TESTING=OFF -DENABLE_DOCUMENTATION=OFF -DUSE_FASTER_LINKER=OFF
                   -DSTATIC_LINK=OFF )
