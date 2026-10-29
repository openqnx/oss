# GNU awk. Version as in the QNX Everywhere 8.0.4r1 image.
VERSION=5.3.2
TARBALL=https://ftp.gnu.org/gnu/gawk/gawk-5.3.2.tar.xz
SHA256=f8c3486509de705192138b00ef2c00bbbdd0e84c30d5c07d23fc73a9dc4cc9cc
DEPS="readline ncurses"
BUILDSYS=autotools
CONF_ARGS_EXTRA=( --disable-nls --without-mpfr )
