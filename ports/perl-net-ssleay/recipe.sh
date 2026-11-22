# perl-net-ssleay: Net::SSLeay, Perl bindings to OpenSSL's libssl and libcrypto (XS).
# Built against this tree's OpenSSL. Makefile.PL takes OPENSSL_PREFIX and
# runs <prefix>/bin/openssl version to learn the version -- a QNX program
# here. The prefix is a directory next to the build directory (not in it:
# MakeMaker's MANIFEST check walks the build tree following links, and
# died in the staging tree's lib/): include/ and lib/ link to the staging
# tree, and bin/openssl is a script that prints the staged headers'
# OPENSSL_VERSION_TEXT (what the target's openssl version prints).
VERSION=1.97_01
TARBALL=https://cpan.metacpan.org/authors/id/C/CH/CHRISN/Net-SSLeay-1.97_01.tar.gz
SHA256=1b6463d0a5b736ecda393310c5e4bd05508314937117b7f49b4c90ecf1a46cde
DEPS="perl openssl zlib"
RUNTIME_DEPS="perl openssl"
BUILDSYS=custom
source "$WS/scripts/lib/perl-module.sh"
do_configure() {
  local o="$B-openssl" v
  v=$(sed -n 's/^# *define OPENSSL_VERSION_TEXT *"\(.*\)"/\1/p' "$STAGE_SYSROOT$PREFIX/include/openssl/opensslv.h")
  [ -n "$v" ] || { echo "no OPENSSL_VERSION_TEXT in the staged opensslv.h"; return 1; }
  mkdir -p "$o/bin" && ln -sfn "$STAGE_SYSROOT$PREFIX/include" "$o/include" && ln -sfn "$STAGE_SYSROOT$PREFIX/lib" "$o/lib" && \
  printf '#!/bin/sh\necho "%s"\n' "$v" > "$o/bin/openssl" && chmod 755 "$o/bin/openssl" && \
  OPENSSL_PREFIX="$o" qnx_perl_xs_configure
}
do_build() { qnx_perl_xs_make; }
do_install() { qnx_perl_xs_install; }
