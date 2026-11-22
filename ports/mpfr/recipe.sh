VERSION=4.2.2
TARBALL=https://ftp.gnu.org/gnu/mpfr/mpfr-4.2.2.tar.xz
SHA256=b67ba0383ef7e8a8563734e2e889ef5ec3c3b898a01d00fa0a6869ad81c6ce01
DEPS="gmp"
BUILDSYS=autotools
CONF_ARGS_EXTRA=( --enable-shared --disable-static --with-gmp="$STAGE$PREFIX" )
