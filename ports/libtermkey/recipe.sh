# libtermkey: keyboard entry processing for terminal programs (key names,
# modifiers, CSI sequences, mouse reports), reading terminfo through
# unibilium. (Upstream has folded it into libtickit; it is still the library
# older neovim/vim plugins and tools link.)
VERSION=0.22
TARBALL=https://www.leonerd.org.uk/code/libtermkey/libtermkey-0.22.tar.gz
SHA256=6945bd3c4aaa83da83d80a045c5563da4edd7d0374c62c0d35aec09eb3014600
DEPS="unibilium"
BUILDSYS=custom
# The Makefile drives everything through the build machine's libtool, which
# knows nothing of the QNX cross compiler: the three sources are compiled
# position-independent with qcc and linked into what libtool would have made
# from -version-info 15:2:14 -- libtermkey.so.1.14.2, soname libtermkey.so.1.
# HAVE_UNIBILIUM as the Makefile sets it when pkg-config finds unibilium.
_tk_cc() { qcc -V$QCC_TARGET -D_QNX_SOURCE -O2 -Wall -std=c99 -DHAVE_UNIBILIUM -I"$STAGE_SYSROOT$PREFIX/include" "$@"; }
do_build() {
  rm -rf "$B" && mkdir -p "$B" && cd "$B" || return 1
  local f
  for f in termkey driver-csi driver-ti; do _tk_cc -fPIC -c "$S/$f.c" -o "$f.o" || return 1; done
  qcc -V$QCC_TARGET -shared -Wl,-soname,libtermkey.so.1 -o libtermkey.so.1.14.2 ./*.o \
    -L"$STAGE_SYSROOT$PREFIX/lib" -lunibilium || return 1
  ln -sf libtermkey.so.1.14.2 libtermkey.so.1 && ln -sf libtermkey.so.1 libtermkey.so || return 1
  # demo: prints each key pressed (the upstream example program)
  _tk_cc -I"$S" -o termkey-demo "$S/demo.c" -L. -ltermkey -L"$STAGE_SYSROOT$PREFIX/lib" -lunibilium
}
do_install() {
  local L="$STAGE$PREFIX/lib"
  install -d "$L/pkgconfig" "$STAGE$PREFIX/include" "$STAGE$PREFIX/bin" &&
  install -m644 "$S/termkey.h" "$STAGE$PREFIX/include/" &&
  install -m755 "$B/libtermkey.so.1.14.2" "$L/" && ln -sf libtermkey.so.1.14.2 "$L/libtermkey.so.1" && ln -sf libtermkey.so.1 "$L/libtermkey.so" &&
  install -m755 "$B/termkey-demo" "$STAGE$PREFIX/bin/" &&
  LIBDIR=$PREFIX/lib INCDIR=$PREFIX/include VERSION=$VERSION sh "$S/termkey.pc.sh" > "$L/pkgconfig/termkey.pc"
}
