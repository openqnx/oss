# Info-ZIP Zip 3.0: creates and updates .zip archives (zip, zipnote,
# zipsplit, zipcloak). Version as in QNX's own package list (the last
# Info-ZIP release). unzip is a separate project.
VERSION=3.0
TARBALL=https://downloads.sourceforge.net/infozip/zip30.tar.gz
SHA256=f0e8bb1f9b7eb0b01285495a2699df3a4b766784c1765a8f1aeedf63c0806369
DEPS=""
# unix/Makefile "generic": unix/configure probes the compiler (compile and
# link tests only) and writes the flags; it builds in the source tree, so
# in a copy (src/ stays clean for scripts/mkpatch). No bzip2 support (zip's
# bzip2 method is rarely used, and it would look for a bzip2 source tree).
# Two of configure's probes run a test program, which a cross build cannot:
# LARGE_FILE_SUPPORT (the answer on QNX 8: 64-bit off_t and st_size, so
# files over 2 GiB and Zip64 archives) is given here; the 16-bit UID/GID
# probe already falls back to the right answer (QNX's are 32-bit).
BUILDSYS=custom
do_build() {
  rm -rf "$B" && mkdir -p "$B" && cp -a "$S/." "$B/" || return 1
  make -C "$B" -f unix/Makefile generic CC="qcc -V$QCC_TARGET -D_QNX_SOURCE" CPP="$QNX_HOST/usr/bin/$QNX_TRIPLE-gcc -E" \
    IZ_BZIP2=/nonexistent LOCAL_ZIP="-O2 -DLARGE_FILE_SUPPORT"
}
do_install() {
  make -C "$B" -f unix/Makefile install prefix="$STAGE$PREFIX" MANDIR="$STAGE$PREFIX/share/man/man1"
}
