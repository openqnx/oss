VERSION=1.0.8; GIT=https://gitlab.com/bzip2/bzip2.git; REF=bzip2-1.0.8
BUILDSYS=custom
# bzip2 1.0.8 predates any CMake support and its Makefile hardcodes the host
# compiler, so drive the compilation directly.
BZ_SRCS="blocksort.c huffman.c crctable.c randtable.c compress.c decompress.c bzlib.c"

do_configure() { :; }

do_build() {
  cd "$B"
  local CC="$QNX_HOST/usr/bin/qcc -V$QCC_TARGET"
  local CFLAGS="-O2 -D_FILE_OFFSET_BITS=64 -D_QNX_SOURCE -fPIC -I$S"
  for f in $BZ_SRCS; do $CC $CFLAGS -c "$S/$f" -o "${f%.c}.o" || return 1; done
  $CC -shared -Wl,-soname,libbz2.so.1.0 -o libbz2.so.1.0.8 ./*.o || return 1
  "$QNX_HOST/usr/bin/nto${QNX_CPU}-ar" rcs libbz2.a ./*.o || return 1
  ln -sf libbz2.so.1.0.8 libbz2.so.1.0
  ln -sf libbz2.so.1.0.8 libbz2.so.1
  ln -sf libbz2.so.1.0.8 libbz2.so
}

do_install() {
  install -d "$STAGE$PREFIX/lib" "$STAGE$PREFIX/include"
  install -m644 "$B/libbz2.so.1.0.8" "$B/libbz2.a" "$STAGE$PREFIX/lib/"
  cp -a "$B/libbz2.so.1.0" "$B/libbz2.so.1" "$B/libbz2.so" "$STAGE$PREFIX/lib/"
  install -m644 "$S/bzlib.h" "$STAGE$PREFIX/include/"
}
