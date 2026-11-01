# NSPR, the Netscape Portable Runtime (threads, I/O, sockets, time) under NSS.
# NSS 3.131.1 needs NSPR 4.39; the QNX changes are qnx-ports' (made for
# 4.38.2, they apply to 4.39 unchanged).
VERSION=4.39
TARBALL=https://ftp.mozilla.org/pub/nspr/releases/v4.39/src/nspr-4.39.tar.gz
SHA256=bbd02ee87a55676063a63e5bc819e0227de2666b47307b2a0134414cdf42368e
BUILDSYS=autotools
# The release archive holds the autoconf project in nspr/.
SUBDIR=nspr
# now and nsinstall run on the build machine during the build: without
# HOST_CC configure compiles them with the cross compiler.
CONF_ENV=( HOST_CC=gcc HOST_CFLAGS=-O2 HOST_LDFLAGS= )
# --target: NSPR's configure picks the machine layer (NTO, _nto.cfg) from
# $target. --enable-64bit: otherwise it assumes a 32-bit target and the
# generated prcpucfg.h contradicts the compiler.
CONF_ARGS_EXTRA=( --target="$QNX_TRIPLE" --enable-64bit --with-pthreads
                  --enable-optimize=-O2 --disable-debug )
# The archive's config.sub (2020) rejects the *-nto-qnx8.0.0 triples; use
# automake's current one (configure itself is shipped generated and kept).
do_patch() {
  local aux; aux=$(ls -d /usr/share/automake-*/ 2>/dev/null | head -1)
  [ -n "$aux" ] || { echo "no automake data directory found"; return 1; }
  cp -f "$aux/config.sub" "$aux/config.guess" "$SRCROOT/nspr/build/autoconf/"
}
