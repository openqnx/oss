VERSION=1.12.4
TARBALL=https://gnupg.org/ftp/gcrypt/libgcrypt/libgcrypt-1.12.4.tar.bz2
SHA256=d77f68f48879510e79a2f65977ccc68981781ea0923e5bdffac2a193ea3d660e
BUILDSYS=autotools
DEPS="libgpg-error"
# configure probes PATH for gpgrt-config and would otherwise pick up the build
# host's copy, which reports the host's libgpg-error rather than ours.
CONF_ENV=( GPGRT_CONFIG="$STAGE$PREFIX/bin/gpgrt-config" )
CONF_ARGS_EXTRA=( --enable-shared --disable-static --disable-doc --disable-tests
                  --with-libgpg-error-prefix="$STAGE$PREFIX" )
