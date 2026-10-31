# GNU Automake. Version as in the QNX Everywhere 8.0.4r1 image.
VERSION=1.18
TARBALL=https://ftp.gnu.org/gnu/automake/automake-1.18.tar.xz
SHA256=5bdccca96b007a7e344c24204b9b9ac12ecd17f5971931a9063bdee4887f4aaf
BUILDSYS=autotools
CONF_ARGS_EXTRA=(  )

# aclocal and automake are generated with the build machine's perl path
# (#!/usr/bin/perl), which a QNX target does not have (hazard 105).
RUNTIME_DEPS="perl autoconf"
do_install() {
  make -C "$B" DESTDIR="$STAGE" install
  local f
  for f in "$STAGE$PREFIX"/bin/*; do sed -i "1s|^#! */usr/bin/perl|#!$PREFIX/bin/perl|" "$f"; done
  # aclocal stops if the system macro directory share/aclocal does not exist.
  # On a target it often does not: it holds only other packages' *.m4 files,
  # which ship in their -dev packages. A missing one is not an error.
  sed -i 's/scan_m4_dirs (FT_SYSTEM, SCAN_M4_DIRS_ERROR, @system_includes);/scan_m4_dirs (FT_SYSTEM, SCAN_M4_DIRS_SILENT, @system_includes);/' \
    "$STAGE$PREFIX/bin/aclocal" "$STAGE$PREFIX/bin/aclocal-1.18"
}
