VERSION=3.1; GIT=https://github.com/npat-efault/picocom.git; REF=3.1
BUILDSYS=custom
do_build() { cp -a "$S/." "$B/" && make -C "$B" CC="qcc -V$QCC_TARGET" CPPFLAGS="-D_QNX_SOURCE -DVERSION_STR=\\\"$VERSION\\\" -DHIGH_BAUD" LDFLAGS=""; }
do_install() { install -d "$STAGE$PREFIX/bin" && install -m755 "$B/picocom" "$STAGE$PREFIX/bin/"; }
