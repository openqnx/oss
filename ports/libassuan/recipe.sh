VERSION=3.0.2
TARBALL=https://gnupg.org/ftp/gcrypt/libassuan/libassuan-3.0.2.tar.bz2
SHA256=d2931cdad266e633510f9970e1a2f346055e351bb19f9b78912475b8074c36f6
BUILDSYS=autotools
DEPS="libgpg-error"
# configure probes PATH for gpgrt-config and would otherwise pick up the build
# host's copy, which reports the host's libgpg-error rather than ours.
CONF_ENV=( GPGRT_CONFIG="$STAGE$PREFIX/bin/gpgrt-config" )
CONF_ARGS_EXTRA=( --enable-shared --disable-static --disable-doc
                  --with-libgpg-error-prefix="$STAGE$PREFIX" )
