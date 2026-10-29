VERSION=1.3.1; TARBALL=https://ftp.gnu.org/gnu/mpc/mpc-1.3.1.tar.gz
SHA256=ab642492f5cf882b74aa0cb730cd410a81edcdbec895183ce930e706c1c759b8
DEPS="gmp mpfr"
BUILDSYS=autotools
CONF_ARGS_EXTRA=( --enable-shared --disable-static
                  --with-gmp="$STAGE$PREFIX" --with-mpfr="$STAGE$PREFIX" )
