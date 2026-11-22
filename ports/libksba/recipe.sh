VERSION=1.9.1
TARBALL=https://gnupg.org/ftp/gcrypt/libksba/libksba-1.9.1.tar.bz2
SHA256=9afdd398b4a076735e1e7fd29c7f6e7670b6c382aee2d3a55aa937b283f43e5b
BUILDSYS=autotools
DEPS="libgpg-error"
# configure probes PATH for gpgrt-config and would otherwise pick up the build
# host's copy, which reports the host's libgpg-error rather than ours.
CONF_ENV=( GPGRT_CONFIG="$STAGE$PREFIX/bin/gpgrt-config" )
CONF_ARGS_EXTRA=( --enable-shared --disable-static --disable-doc
                  --with-libgpg-error-prefix="$STAGE$PREFIX" )
