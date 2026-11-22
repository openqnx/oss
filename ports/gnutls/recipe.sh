VERSION=3.8.14
TARBALL=https://www.gnupg.org/ftp/gcrypt/gnutls/v3.8/gnutls-3.8.14.tar.xz
SHA256=7e2d258071f5db9620aa7ff34259331d004f7beafa3eb18560cfaf60acfbf02a
BUILDSYS=autotools
# Also linked at run time (found by scripts/gen-dependency-index): brotli zstd
DEPS="nettle gmp libtasn1 libunistring libidn2 zlib brotli zstd"
CONF_ARGS_EXTRA=( --enable-shared --disable-static --disable-doc --disable-tests
                  --disable-tools --disable-cxx --without-p11-kit --with-included-libtasn1=no
                  --disable-guile --disable-nls --with-included-unistring=no )
# 3.8.12 includes <link.h> where configure found dl_iterate_phdr; QNX has the
# function but declares it in <sys/link.h>. gnutls uses it only for the FIPS
# self-check of its own library file (not enabled here): say it is missing.
CONF_ENV=( ac_cv_func_dl_iterate_phdr=no )
