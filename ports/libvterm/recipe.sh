# libvterm: an abstract VT220/xterm/ECMA-48 terminal emulator library (the
# one vim and neovim embed), with its vterm-ctrl/vterm-dump/unterm tools.
VERSION=0.3.3
TARBALL=https://www.leonerd.org.uk/code/libvterm/libvterm-0.3.3.tar.gz
SHA256=09156f43dd2128bd347cbeebe50d9a571d32c64e0cf18d211197946aff7226e0
BUILDSYS=custom
# The Makefile drives everything through the build machine's libtool, which
# knows nothing of the QNX cross compiler. The release tarball already has the
# generated tables (src/encoding/*.inc, src/fullwidth.inc), so the library is
# just src/*.c: compiled position-independent with qcc and linked into what
# libtool would have made from -version-info 0:0:0 -- libvterm.so.0.0.0,
# soname libvterm.so.0.
_vt_cc() { qcc -V$QCC_TARGET -D_QNX_SOURCE -O2 -Wall -std=c99 -I"$S/include" "$@"; }
do_build() {
  rm -rf "$B" && mkdir -p "$B" && cd "$B" || return 1
  local f
  for f in "$S"/src/*.c; do _vt_cc -fPIC -c "$f" -o "$(basename "${f%.c}").o" || return 1; done
  qcc -V$QCC_TARGET -shared -Wl,-soname,libvterm.so.0 -o libvterm.so.0.0.0 ./*.o || return 1
  ln -sf libvterm.so.0.0.0 libvterm.so.0 && ln -sf libvterm.so.0 libvterm.so || return 1
  for f in unterm vterm-ctrl vterm-dump; do _vt_cc -o "$f" "$S/bin/$f.c" -L. -lvterm || return 1; done
}
do_install() {
  local L="$STAGE$PREFIX/lib"
  install -d "$L/pkgconfig" "$STAGE$PREFIX/include" "$STAGE$PREFIX/bin" &&
  install -m644 "$S"/include/*.h "$STAGE$PREFIX/include/" &&
  install -m755 "$B/libvterm.so.0.0.0" "$L/" && ln -sf libvterm.so.0.0.0 "$L/libvterm.so.0" && ln -sf libvterm.so.0 "$L/libvterm.so" &&
  install -m755 "$B/unterm" "$B/vterm-ctrl" "$B/vterm-dump" "$STAGE$PREFIX/bin/" &&
  sed -e "s,@INCDIR@,$PREFIX/include," -e "s,@LIBDIR@,$PREFIX/lib," -e "s,@VERSION@,$VERSION," "$S/vterm.pc.in" > "$L/pkgconfig/vterm.pc"
}
