# GPGME, GnuPG Made Easy: the C library through which applications use GnuPG
# (it runs gpg, gpgsm and gpgconf from the gnupg component and talks to them
# over pipes and Assuan).
VERSION=2.2.0
TARBALL=https://gnupg.org/ftp/gcrypt/gpgme/gpgme-2.2.0.tar.bz2
SHA256=7160e80e84dafd00d956c84891c533bb7ab16a6a54fbe1574b2f3acf0496977b
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
