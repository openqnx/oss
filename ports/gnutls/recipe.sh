VERSION=3.8.8
TARBALL=https://www.gnupg.org/ftp/gcrypt/gnutls/v3.8/gnutls-3.8.8.tar.xz
SHA256=ac4f020e583880b51380ed226e59033244bc536cad2623f2e26f5afa2939d8fb
BUILDSYS=autotools
# Also linked at run time (found by scripts/gen-dependency-index): brotli zstd
DEPS="nettle gmp libtasn1 libunistring libidn2 zlib brotli zstd"
CONF_ARGS_EXTRA=( --enable-shared --disable-static --disable-doc --disable-tests
                  --disable-tools --disable-cxx --without-p11-kit --with-included-libtasn1=no
                  --disable-guile --disable-nls --with-included-unistring=no )
