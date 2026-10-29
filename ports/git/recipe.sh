# Git for the target. Version as in the QNX Everywhere 8.0.4r1 image.
VERSION=2.51.0
TARBALL=https://mirrors.edge.kernel.org/pub/software/scm/git/git-2.51.0.tar.xz
SHA256=60a7c2251cc2e588d5cd87bae567260617c6de0c22dca9cdbfc4c7d2b8990b62
DEPS="zlib openssl curl expat pcre2 perl perl-error libiconv"
BUILDSYS=custom
# Git's Makefile is configured by variables rather than a configure script.
# uname_S=QNX selects its QNX settings; the rest names the target's paths and
# the features that are cross compiled here.
# git builds its own link list (EXTLIBS: -lz, -lcrypto, ...); setting EXTLIBS
# here replaced it ("undefined reference to crc32"). NEEDS_SOCKET adds QNX's
# -lsocket to it, and NO_REGEX builds git's bundled regex, so no -lregex.
# The staged GNU libiconv's <iconv.h> renames iconv() to libiconv(), which
# QNX's libc lacks: NEEDS_LIBICONV links -liconv.
_gitmake() { make -C "$B" -j"$(nproc)" prefix="$PREFIX" CC="qcc -V$QCC_TARGET" AR="$QNX_HOST/usr/bin/nto${QNX_CPU}-ar" \
  uname_S=QNX uname_M="$QNX_CPU" CFLAGS="-O2 -D_QNX_SOURCE -I$STAGE_SYSROOT$PREFIX/include" LDFLAGS="-L$STAGE_SYSROOT$PREFIX/lib" \
  NO_GETTEXT=1 NO_TCLTK=1 NO_PYTHON=1 USE_LIBPCRE2=1 CURL_CONFIG=false CURL_LDFLAGS="-lcurl" \
  PERL_PATH=/usr/local/bin/perl SHELL_PATH=/bin/sh NO_REGEX=NeedsStartEnd HOST_CPU="$QNX_CPU" \
  NEEDS_SOCKET=YesPlease NEEDS_LIBICONV=YesPlease "$@"; }
do_configure() { cp -a "$S/." "$B/"; }
do_build() { _gitmake all; }
do_install() { _gitmake DESTDIR="$STAGE" install; }
