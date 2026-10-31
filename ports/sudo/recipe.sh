# sudo. Version as in the QNX Everywhere 8.0.4r1 image.
VERSION=1.9.17p2
# Also linked at run time (found by scripts/gen-dependency-index): libxcrypt openssl zlib
DEPS="libxcrypt openssl zlib"
TARBALL=https://www.sudo.ws/dist/sudo-1.9.17p2.tar.gz
SHA256=4a38a1ab3adb1199257edc2a7c4a2bd714665eb605b04368843b06dada2cfcfb
BUILDSYS=autotools
# QNX's <signal.h> has SA_RESTART commented out ("not supported yet"); sudo
# only asks for it as a convenience.
CFLAGS_APPEND="-DSA_RESTART=0"
# QNX's libc has dl_iterate_phdr() but no <link.h>; sudo only uses it as one
# more entropy source.
CONF_ENV=( ac_cv_func_dl_iterate_phdr=no )
# libsudo_util uses regcomp()/regexec(), which QNX keeps in libregex. Not via
# LIBS: configure also links its build-machine compiler test with LIBS, and
# the build machine has no libregex.
LDFLAGS_APPEND="-lregex"
# sysconfdir under the prefix: packages install into the apk root
# (/data/oss), so a sudoers in /etc would never be the one shipped.
CONF_ARGS_EXTRA=( --disable-nls --without-pam --with-env-editor --sysconfdir="$PREFIX/etc" )
# sudo's install chowns its files to root, which an unprivileged build cannot
# do ("Operation not permitted"). Install without the owner flags: the modes,
# setuid included, are kept, and scripts/qnxpkg records every file as root.
do_install() { make -C "$B" DESTDIR="$STAGE" INSTALL_OWNER= install; }
