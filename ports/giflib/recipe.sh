VERSION=5.2.2
TARBALL=https://downloads.sourceforge.net/giflib/giflib-5.2.2.tar.gz
SHA256=be7ffbd057cadebe2aa144542fd90c6838c6a083b5e8a9048b8ee3b66b29d5fb
BUILDSYS=custom
# giflib ships only a Makefile that assumes the host toolchain.
do_configure() { :; }
do_build() {
  cd "$B"
  local CC="$QNX_HOST/usr/bin/qcc -V$QCC_TARGET"
  for f in dgif_lib egif_lib gifalloc gif_err gif_font gif_hash openbsd-reallocarray; do
    $CC -O2 -fPIC -D_QNX_SOURCE -I"$S" -c "$S/$f.c" -o "$f.o" || return 1
  done
  $CC -shared -Wl,-soname,libgif.so.7 -o libgif.so.7.2.0 ./*.o || return 1
  ln -sf libgif.so.7.2.0 libgif.so.7; ln -sf libgif.so.7.2.0 libgif.so
}
do_install() {
  install -d "$STAGE$PREFIX/lib" "$STAGE$PREFIX/include"
  install -m644 "$B/libgif.so.7.2.0" "$STAGE$PREFIX/lib/"
  cp -a "$B/libgif.so.7" "$B/libgif.so" "$STAGE$PREFIX/lib/"
  install -m644 "$S/gif_lib.h" "$STAGE$PREFIX/include/"
}
