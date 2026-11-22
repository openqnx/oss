# GNU awk. Version as in the QNX Everywhere 8.0.4r1 image.
VERSION=5.4.2
TARBALL=https://ftp.gnu.org/gnu/gawk/gawk-5.4.2.tar.xz
SHA256=b95e4909533b5f988c6b5e5539b3b3ddf76e7d7df02ca2e5b5b7d6f62db78e6a
DEPS="readline ncurses"
BUILDSYS=autotools
CONF_ARGS_EXTRA=( --disable-nls --without-mpfr )
