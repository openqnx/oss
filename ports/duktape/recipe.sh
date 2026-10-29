VERSION=2.7.0
TARBALL=https://duktape.org/duktape-2.7.0.tar.xz
SHA256=90f8d2fa8b5567c6899830ddef2c03f3c27960b11aca222fa17aa7ac613c2890
BUILDSYS=custom
# Duktape ships a prepared amalgamation rather than a build system.
do_build() {
  "$QNX_HOST/usr/bin/qcc" -V"$QCC_TARGET" -O2 -fPIC -D_QNX_SOURCE \
    -I"$S/src" -c "$S/src/duktape.c" -o "$B/duktape.o"
  "$QNX_HOST/usr/bin/qcc" -V"$QCC_TARGET" -shared -Wl,-soname,libduktape.so.207 \
    -o "$B/libduktape.so.207.0.0" "$B/duktape.o" -lm
}
do_install() {
  install -d "$STAGE$PREFIX/lib" "$STAGE$PREFIX/include"
  install -m 755 "$B/libduktape.so.207.0.0" "$STAGE$PREFIX/lib/"
  ln -sfn libduktape.so.207.0.0 "$STAGE$PREFIX/lib/libduktape.so.207"
  ln -sfn libduktape.so.207     "$STAGE$PREFIX/lib/libduktape.so"
  install -m 644 "$S/src/duktape.h" "$S/src/duk_config.h" "$STAGE$PREFIX/include/"
}
