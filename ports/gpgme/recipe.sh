# GPGME, GnuPG Made Easy: the C library through which applications use GnuPG
# (it runs gpg, gpgsm and gpgconf from the gnupg component and talks to them
# over pipes and Assuan).
VERSION=2.3.1
TARBALL=https://gnupg.org/ftp/gcrypt/gpgme/gpgme-2.3.1.tar.bz2
SHA256=2bf94e0e677e5a8fc31f79dd323f91c5416b36fb851058bc1e8bce39f2642434
BUILDSYS=autotools
DEPS="libgpg-error libassuan"
RUNTIME_DEPS="gnupg"
# configure probes PATH for gpgrt-config and would otherwise pick up the build
# host's copy, which reports the host's libgpg-error rather than ours.
CONF_ENV=( GPGRT_CONFIG="$STAGE$PREFIX/bin/gpgrt-config" )
CONF_ARGS_EXTRA=( --enable-shared --disable-static --disable-gpg-test --disable-gpgsm-test
                  --disable-gpgconf-test --disable-g13-test
                  --with-libgpg-error-prefix="$STAGE$PREFIX"
                  --with-libassuan-prefix="$STAGE$PREFIX" )
