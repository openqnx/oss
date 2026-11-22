# GNU Autoconf (configure script generator; needs perl and m4 on the target). Version as in the QNX Everywhere 8.0.4r1 image.
VERSION=2.73
TARBALL=https://ftp.gnu.org/gnu/autoconf/autoconf-2.73.tar.xz
SHA256=9fd672b1c8425fac2fa67fa0477b990987268b90ff36d5f016dae57be0d6b52e
BUILDSYS=autotools
CONF_ARGS_EXTRA=(  )

# The scripts are generated with the build machine's perl and m4 paths
# (#! /usr/bin/perl, and the GNU m4 autom4te runs). A QNX target has neither:
# point them at the installed perl and m4 (hazard 105).
RUNTIME_DEPS="perl m4"
do_install() {
  make -C "$B" DESTDIR="$STAGE" install
  local f
  for f in "$STAGE$PREFIX"/bin/*; do
    sed -i -e "1s|^#! */usr/bin/perl|#!$PREFIX/bin/perl|" -e "s|exec /usr/bin/perl |exec $PREFIX/bin/perl |g" \
      -e "/^my \$m4 = /s|'[^']*';|'$PREFIX/bin/m4';|" "$f"
  done
}
