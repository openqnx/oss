# GNU Automake. Version as in the QNX Everywhere 8.0.4r1 image.
VERSION=1.20
TARBALL=https://ftp.gnu.org/gnu/automake/automake-1.20.tar.xz
SHA256=f6f34dad4cbe395715762b0710f7c511e39bd5cf777e6b95818ccbecf7714248
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
    "$STAGE$PREFIX/bin/aclocal" "$STAGE$PREFIX/bin/aclocal-$(echo "$VERSION" | cut -d. -f1,2)"
}
