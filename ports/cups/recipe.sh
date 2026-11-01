# CUPS, the printing system: libcups and libcupsimage, the scheduler (cupsd),
# the command-line tools (lp, lpr, lpstat, lpadmin, cancel, cupsctl, ipptool,
# ippfind, ippeveprinter ...), and the backends and filters. 2.4.16 with
# qnx-ports' QNX support (patches/0001: s6_addr32 is spelled
# __u6_addr.__u6_addr32 in QNX's <netinet6/in6.h>, and programs link libcups
# together with its own libraries); patches/0002 lets cupsd pass
# LD_LIBRARY_PATH on to its filters and backends (see below).
VERSION=2.4.16; GIT=https://github.com/OpenPrinting/cups.git; REF=v2.4.16
# libxcrypt: crypt() for cupsd's Basic authentication against local accounts
# (configure links -lcrypt, which QNX's libc does not provide).
DEPS="openssl zlib avahi libiconv libxcrypt"
BUILDSYS=custom
# patches/0001 changes config-scripts/cups-sharedlibs.m4 (programs link
# libcups together with its own libraries: QNX's GNU ld does not resolve a
# program's direct references, regexec from libregex say, through libcups's
# DT_NEEDED); the shipped configure is regenerated from it.
do_patch() { ( cd "$SRCROOT" && autoconf -f ); }
# CUPS's makefiles build in the source tree only (Makedefs at the top), so
# configure and build in a copy of it, with the same compiler settings the
# autotools driver uses.
# TLS: OpenSSL. DNS-SD: Avahi (printer sharing and ippfind). Off: PAM and
# Kerberos (neither exists on QNX), libusb (no USB backend: QNX's USB stack is
# not libusb's), systemd/launchd on-demand start, D-Bus notifications, the
# translations of the web interface (--with-languages=none).
# -z stack-size: ipptool's do_test() keeps a 1 MiB copy buffer on the stack,
# far more than QNX gives the main thread by default (hazard 119): it died
# with SIGSEGV on its first request. 8 MiB, Linux's default, for every
# program here (with -z noexecstack: the stack-size header would otherwise
# come out executable).
# -rpath $PREFIX/lib (DT_RPATH, which covers the libraries' own
# dependencies too): cupsd runs its filters, backends and helpers with a
# clean environment and, as root, refuses to pass any LD_* variable on
# (scheduler/env.c). /usr/local/lib is not on QNX's default library path, so
# every filter failed to start ("Could not load library libcups.so.2"). The
# loader still takes LD_LIBRARY_PATH first where it is set -- and with
# patches/0002 cupsd passes its own LD_LIBRARY_PATH on, which a minimal
# image needs for QNX's libeventfd & co. in a directory of their own.
# -lregex: regcomp()/regexec() (ipptool, the scheduler) are in QNX's libregex,
# not libc.
# State (spool, cache, logs, run) under $PREFIX/var; the icon and desktop
# file under the prefix too (CUPS's defaults are /usr/share/...).
do_configure() {
  cp -a "$S/." "$B/" && cd "$B" && \
  CC="qcc -V$QCC_TARGET -D_QNX_SOURCE" CXX="q++ -V${QCC_TARGET}_cxx -D_QNX_SOURCE" \
  CPP="$QNX_HOST/usr/bin/$QNX_TRIPLE-gcc -E" \
  CPPFLAGS="-D_QNX_SOURCE -I$STAGE$PREFIX/include" \
  CFLAGS="-D_QNX_SOURCE -I$STAGE$PREFIX/include" CXXFLAGS="-D_QNX_SOURCE -I$STAGE$PREFIX/include" \
  LDFLAGS="-Wl,--push-state,--no-as-needed -lc -Wl,--pop-state -L$STAGE$PREFIX/lib -Wl,-rpath-link,$STAGE$PREFIX/lib -Wl,-z,stack-size=8388608 -Wl,-z,noexecstack -Wl,-rpath,$PREFIX/lib -Wl,--disable-new-dtags" \
  LIBS="-lsocket -lregex" \
  ./configure --host="$QNX_TRIPLE" --prefix="$PREFIX" --sysconfdir="$PREFIX/etc" --localstatedir="$PREFIX/var" \
    --with-tls=openssl --with-dnssd=avahi --disable-dbus --disable-pam --disable-gssapi --disable-libusb \
    --disable-libpaper --disable-acl --with-ondemand=no --without-systemd --without-rcdir \
    --with-languages=none --disable-static --enable-shared \
    --with-icondir="$PREFIX/share/icons" --with-menudir="$PREFIX/share/applications"
}
do_build() { make -C "$B" -j"$(nproc)"; }
# BUILDROOT is CUPS's DESTDIR.
do_install() { make -C "$B" BUILDROOT="$STAGE" install; }
