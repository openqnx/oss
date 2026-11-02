# 7-Zip 25.01: the 7zz command-line archiver (7z, xz, zip, tar, gzip,
# bzip2 ... create, list, test, extract; also reads many more formats), with
# qnx-ports' QNX support (patches/0001): no d_type in QNX's struct dirent
# (directory entries are typed through stat, as on AIX and Solaris), the
# RAM size from sysconf(_SC_PHYS_PAGES), no pthread affinity calls, and
# makefile changes so the cross build passes its own CFLAGS and is not
# stripped or built with -Werror. patches/0002: no getauxval() hardware
# probe on QNX (arm64); 0003: the RAM size from the system page, since QNX
# does not implement _SC_PHYS_PAGES (0001 reported 0 bytes).
VERSION=25.01
TARBALL=https://github.com/ip7z/7zip/releases/download/25.01/7z2501-src.tar.xz
SHA256=ed087f83ee789c1ea5f39c464c55a5c9d4008deb0efe900814f2df262b82c36e
BUILDSYS=custom
# The source archive has no top-level directory, so the default fetch
# (--strip-components=1) would drop one level: unpack it as is.
do_fetch() {
  local tb="$WS/dist/7z2501-src.tar.xz"; mkdir -p "$WS/dist" "$SRCROOT"
  [ -f "$tb" ] || curl -fsSL -o "$tb" "$TARBALL" || return 1
  echo "$SHA256  $tb" | sha256sum -c --status || { echo "7zip checksum mismatch"; return 1; }
  tar xJf "$tb" -C "$SRCROOT"
}
# Bundles/Alone2 is 7zz, the standalone 7-Zip with every codec built in (no
# 7z.so plugin). cmpl_gcc.mak: the portable C/C++ code, no assembler (the
# x86 .asm files need asmc/uasm). The patched makefile appends to CFLAGS but
# sets CXXFLAGS itself, so the optimisation goes in MY_ARCH, which both use.
# "-z noexecstack" goes through -Wl: qcc does not forward a bare -z.
# LIB2 (-lpthread -ldl) is empty: both are part of QNX's libc.
_7m() { CFLAGS="-D_QNX_SOURCE" make -C "$B/CPP/7zip/Bundles/Alone2" -f ../../cmpl_gcc.mak \
  CC="qcc -V$QCC_TARGET" CXX="q++ -V${QCC_TARGET}_cxx" MY_ARCH="-O2 -D_QNX_SOURCE" \
  LFLAGS_NOEXECSTACK="-Wl,-z,noexecstack" LIB2= "$@"; }
do_configure() { cp -a "$S/." "$B/"; }
do_build() { _7m -j"$(nproc)"; }
do_install() { install -d "$STAGE$PREFIX/bin" && install -m755 "$B/CPP/7zip/Bundles/Alone2/b/g/7zz" "$STAGE$PREFIX/bin/7zz"; }
