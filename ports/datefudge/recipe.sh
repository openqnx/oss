# datefudge: runs a program with a faked current time -- an LD_PRELOAD
# library that shifts what time(), gettimeofday() and clock_gettime()
# (CLOCK_REALTIME) return, and the datefudge wrapper script. Version as in
# QNX's own package list (Debian's upstream).
VERSION=1.24
TARBALL=https://deb.debian.org/debian/pool/main/d/datefudge/datefudge_1.24.tar.xz
SHA256=80efab249232deeb142bf8a0ec66f2cf596c45560e4fdd28f3b5bdd290ac0385
DEPS=""
# QNX's runtime linker honours LD_PRELOAD, and a preloaded library's
# definitions interpose on the program's calls as on Linux. Calls libc
# makes internally (strftime, localtime, ...) are not affected -- the same
# as with glibc. The wrapper runs "date -d" to parse the date: QNX's
# /system/bin/date (toybox) accepts it.
BUILDSYS=custom
# The Makefile builds in its directory: a copy. VERSION= because it would
# take the version from dpkg-parsechangelog. The wrapper (datefudge.sh with
# its paths filled in) and the manual come from the Makefile; the library is
# linked here: the Makefile links it with -ldl, and QNX has dlopen/dlsym in
# libc and no libdl. It goes to $PREFIX/lib/datefudge/datefudge.so, where the
# installed wrapper looks for it.
do_build() {
  rm -rf "$B" && mkdir -p "$B" && cp -a "$S/." "$B/" || return 1
  make -C "$B" VERSION=$VERSION prefix="$PREFIX" datefudge datefudge.1 || return 1
  qcc -V$QCC_TARGET -D_QNX_SOURCE -O2 -Wall -Wextra -fpic -shared -o "$B/datefudge.so" "$B/datefudge.c"
}
# Installed by hand: "make install" depends on datefudge.o, which this build
# never makes, so make would compile and link datefudge.so again -- with the
# build machine's cc (a Linux library in the package).
do_install() {
  install -d "$STAGE$PREFIX/lib/datefudge" "$STAGE$PREFIX/bin" "$STAGE$PREFIX/share/man/man1" || return 1
  install -m 644 "$B/datefudge.so" "$STAGE$PREFIX/lib/datefudge/datefudge.so" &&
  install -m 755 "$B/datefudge" "$STAGE$PREFIX/bin/datefudge" &&
  install -m 644 "$B/datefudge.1" "$STAGE$PREFIX/share/man/man1/datefudge.1"
}
