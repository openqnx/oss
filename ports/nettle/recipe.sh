VERSION=3.10.1
TARBALL=https://ftp.gnu.org/gnu/nettle/nettle-3.10.1.tar.gz
SHA256=b0fcdd7fc0cdea6e80dcf1dd85ba794af0d5b4a57e26397eee3bc193272d9132
BUILDSYS=autotools
DEPS="gmp"
CONF_ARGS_EXTRA=( --enable-shared --disable-static --disable-documentation
                  --with-lib-path="$STAGE$PREFIX/lib" --with-include-path="$STAGE$PREFIX/include" )
