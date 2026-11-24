# 2.4 is the newest release series this tree's libraries can carry: GnuPG 2.5
# needs libgpg-error >= 1.56 and libgcrypt >= 1.12 (ported here: 1.50, 1.11).
VERSION=2.4.9
TARBALL=https://gnupg.org/ftp/gcrypt/gnupg/gnupg-2.4.9.tar.bz2
SHA256=dd17ab2e9a04fd79d39d853f599cbc852062ddb9ab52a4ddeb4176fd8b302964
BUILDSYS=autotools
DEPS="libgpg-error libgcrypt libassuan libksba npth gnutls sqlite zlib bzip2 readline libiconv"
# configure probes PATH for gpgrt-config and would otherwise pick up the build
# host's copy, which reports the host's libgpg-error rather than ours.
CONF_ENV=( GPGRT_CONFIG="$STAGE$PREFIX/bin/gpgrt-config" )
# No LDAP client library and no TPM2 stack here: dirmngr goes without LDAP
# keyserver support, and tpm2d is not built. The internal CCID driver needs
# libusb; scdaemon still talks to readers through PC/SC (loaded at run time).
# NLS off: messages stay English, and no catalogue path is compiled in.
# --disable-tests: the test-suite interpreter (gpgscm) needs gcc's -imacros,
# which qcc rejects, and the suite only runs on the build machine anyway.
CONF_ARGS_EXTRA=( --disable-doc --disable-ldap --disable-tpm2d --disable-ccid-driver
                  --disable-nls --disable-tests --enable-sqlite --enable-gnutls
                  --with-libgpg-error-prefix="$STAGE$PREFIX"
                  --with-libgcrypt-prefix="$STAGE$PREFIX"
                  --with-libassuan-prefix="$STAGE$PREFIX"
                  --with-ksba-prefix="$STAGE$PREFIX"
                  --with-npth-prefix="$STAGE$PREFIX"
                  --with-zlib="$STAGE$PREFIX" --with-bzip2="$STAGE$PREFIX"
                  --with-readline="$STAGE$PREFIX" )
