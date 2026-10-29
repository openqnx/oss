VERSION=4.2.1; TARBALL=https://ftp.gnu.org/gnu/mpfr/mpfr-4.2.1.tar.xz
SHA256=277807353a6726978996945af13e52829e3abd7a9a5b7fb2793894e18f1fcbb2
DEPS="gmp"
BUILDSYS=autotools
CONF_ARGS_EXTRA=( --enable-shared --disable-static --with-gmp="$STAGE$PREFIX" )
