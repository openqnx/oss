# NASM, the Netwide Assembler, and ndisasm: an x86/x86-64 assembler (Intel
# syntax; ELF, COFF, Mach-O, binary and other output formats). Version as in
# QNX's own package list. It runs on both architectures (on aarch64 it is
# a cross assembler for x86 code).
VERSION=2.16.03
TARBALL=https://www.nasm.us/pub/nasm/releasebuilds/2.16.03/nasm-2.16.03.tar.xz
SHA256=1412a1c760bbd05db026b6c0d1657affd6631cd0a63cddb6f73cc6d4aa616148
DEPS=""
# Autotools, configured by hand: the default configure step puts the
# staging include directory first (qnxbuild), where a staged md5.h (libmd's) shadows
# NASM's own include/md5.h (a different MD5Transform), hazard 22. NASM needs
# nothing from staging, so its compiler sees only the SDP's headers.
BUILDSYS=custom
do_configure() {
  ( cd "$B" && CC="qcc -V$QCC_TARGET -D_QNX_SOURCE" \
      CPP="$QNX_HOST/usr/bin/$QNX_TRIPLE-gcc -E" CFLAGS="-O2" \
      "$S/configure" --host="$QNX_TRIPLE" --prefix="$PREFIX" )
}
do_build() { make -C "$B" -j"$(nproc)"; }
# The manuals are prebuilt in the release tarball (nasm.1, ndisasm.1).
do_install() { make -C "$B" DESTDIR="$STAGE" install; }
