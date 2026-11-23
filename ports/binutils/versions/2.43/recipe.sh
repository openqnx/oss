# GNU binutils for the target (as, ld, objdump, readelf, ...). Version as in the QNX Everywhere 8.0.4r1 image.
VERSION=2.43
TARBALL=https://ftp.gnu.org/gnu/binutils/binutils-2.43.tar.xz
SHA256=b53606f443ac8f01d1d5fc9c39497f2af322d99e14cea5c0b4b124d630379365
# Also linked at run time (found by scripts/gen-dependency-index): zstd
DEPS="zlib zstd msgpack-c"
BUILDSYS=autotools
CONF_ARGS_EXTRA=( --disable-nls --disable-werror --enable-shared=no --with-system-zlib --disable-gprofng )
# binutils' own include/ goes ahead of the staging include directory: libmd's
# staged md5.h (and its sha1.h) otherwise hides include/md5.h, and libiberty's
# md5.c fails on an undefined struct md5_ctx (hazard 22; staged before binutils
# in the clean SDP 8.0.6 build order).
CONF_ENV=( CPPFLAGS="-I$SRC_DIR/$NAME/include -D_QNX_SOURCE -I$STAGE$PREFIX/include -I$QNX_TARGET/usr/include"
  CFLAGS="-I$SRC_DIR/$NAME/include -D_QNX_SOURCE -I$STAGE$PREFIX/include -I$QNX_TARGET/usr/include" )
