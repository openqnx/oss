VERSION=1.4.1
TARBALL=https://ftp.gnu.org/gnu/mpc/mpc-1.4.1.tar.xz
SHA256=91204cd32f164bd3b7c992d4a6a8ce6519511aadab30f78b6982d0bf8d73e931
DEPS="gmp mpfr"
BUILDSYS=autotools
CONF_ARGS_EXTRA=( --enable-shared --disable-static
                  --with-gmp="$STAGE$PREFIX" --with-mpfr="$STAGE$PREFIX" )
