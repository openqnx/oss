VERSION=1.8.1
TARBALL=https://gnupg.org/ftp/gcrypt/libksba/libksba-1.8.1.tar.bz2
SHA256=c2f84393011827219ae117131dba8e7684c2bed0961eed11b0642c2acba440b5
BUILDSYS=autotools
DEPS="libgpg-error"
# configure probes PATH for gpgrt-config and would otherwise pick up the build
# host's copy, which reports the host's libgpg-error rather than ours.
CONF_ENV=( GPGRT_CONFIG="$STAGE$PREFIX/bin/gpgrt-config" )
CONF_ARGS_EXTRA=( --enable-shared --disable-static --disable-doc
                  --with-libgpg-error-prefix="$STAGE$PREFIX" )
