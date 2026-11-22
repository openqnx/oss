VERSION=1.0.24
TARBALL=https://www.quut.com/gsm/gsm-1.0.24.tar.gz
SHA256=a3c40c6471928383f4abfcb2e8f24012a1f562be2f17b8d672145d5986681a92
BUILDSYS=custom
# The upstream Makefile hardcodes the host compiler and installs nowhere useful;
# the library is eight objects, so build it directly.
do_build() {
  mkdir -p "$B/o"
  for f in "$S"/src/*.c; do
    "$QNX_HOST/usr/bin/qcc" -V"$QCC_TARGET" -O2 -fPIC -D_QNX_SOURCE \
      -DSASR -DWAV49 -DNeedFunctionPrototypes=1 -I"$S/inc" \
      -c "$f" -o "$B/o/$(basename "${f%.c}").o"
  done
  "$QNX_HOST/usr/bin/qcc" -V"$QCC_TARGET" -shared -Wl,-soname,libgsm.so.1 \
    -o "$B/libgsm.so.1.0.22" "$B"/o/*.o
}
do_install() {
  install -d "$STAGE$PREFIX/lib" "$STAGE$PREFIX/include/gsm"
  install -m 755 "$B/libgsm.so.1.0.22" "$STAGE$PREFIX/lib/"
  ln -sfn libgsm.so.1.0.22 "$STAGE$PREFIX/lib/libgsm.so.1"
  ln -sfn libgsm.so.1      "$STAGE$PREFIX/lib/libgsm.so"
  install -m 644 "$S/inc/gsm.h" "$STAGE$PREFIX/include/gsm/"
  install -m 644 "$S/inc/gsm.h" "$STAGE$PREFIX/include/"
}
