# GNU binutils for the target (as, ld, objdump, readelf, ...). Version as in the QNX Everywhere 8.0.4r1 image.
VERSION=2.43
TARBALL=https://ftp.gnu.org/gnu/binutils/binutils-2.43.tar.xz
SHA256=b53606f443ac8f01d1d5fc9c39497f2af322d99e14cea5c0b4b124d630379365
# Also linked at run time (found by scripts/gen-dependency-index): zstd
DEPS="zlib zstd"
BUILDSYS=autotools
CONF_ARGS_EXTRA=( --disable-nls --disable-werror --enable-shared=no --with-system-zlib --disable-gprofng )
