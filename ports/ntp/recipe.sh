# The reference NTP implementation (ntpd, ntpq, ntpdc, ntpdate, ...) from
# ntp.org / Network Time Foundation.
VERSION=4.2.8p18
TARBALL=https://downloads.nwtime.org/ntp/4.2.8/ntp-4.2.8p18.tar.gz
SHA256=cf84c5f3fb1a295284942624d823fffa634144e096cfc4f9969ac98ef5f468e5
BUILDSYS=autotools
# ntp-keygen and ntpd use libevent 2 (found with pkg-config), ntpq/ntpdc
# libedit for their command line.
DEPS="libevent libedit"
# ntp-wait, ntptrace and update-leap are Perl scripts (#!/usr/local/bin/perl);
# no DT_NEEDED shows that.
RUNTIME_DEPS="perl"
# NTPsec (a fork of this code) is ported too and installs ntpd, ntpq,
# ntptrace and ntpwait under the same names. The reference implementation
# goes in a prefix of its own, /usr/local/opt/ntp (bin/ sbin/ share/man/),
# so both can be installed side by side.
PREFIX_NTP="$PREFIX/opt/ntp"
# Without OpenSSL: symmetric keys still work (ntp's own MD5); Autokey and
# SHA/AES-CMAC keys need it. Without sntp, which needs the bundled libevent
# (ntpdate covers the one-shot query).
# No signalled I/O: configure enables SIGIO-driven input on QNX, and ntpd
# then exits at startup with "ioctl(SIOCSPGRP) fails: Operation not
# supported" (io-sock does not take SIOCSPGRP); ntpd's select() loop
# serves the sockets instead.
CONF_ARGS_EXTRA=( --prefix="$PREFIX_NTP" --disable-signalled-io --without-crypto --without-sntp --disable-linuxcaps
                  --without-ntpsnmpd --with-yielding-select=yes --with-lineeditlibs=edit )
# ntp-wait, ntptrace, update-leap and calc_tickadj are Perl scripts: their
# #! line names the target's perl (the perl port, /usr/local/bin/perl), not
# the build machine's /usr/bin/perl. Perl is needed only for those four.
# Configure would also take whatever else staging holds: libmd for MD5 (ntp
# has its own), Avahi's libdns_sd for mDNS registration, gettext's libintl,
# and a libelf for nlist() (only tickadj, not built here, uses it). None of
# them is needed: they are answered "no", so the binaries depend on exactly
# what DEPS says.
CONF_ENV=( ac_cv_path_PATH_PERL=/usr/local/bin/perl ac_cv_search_MD5Init=no
           ac_cv_header_dns_sd_h=no ac_cv_search_DNSServiceRegister=no
           ac_cv_lib_intl_gettext=no ac_cv_search_nlist=no ac_cv_header_nlist_h=no )
# tickadj (and calc_tickadj) read and patch the kernel's tick variables
# through /dev/kmem and nlist(): there is no such thing on QNX (the tick
# is ClockPeriod()'s), so they are neither built nor installed.
_ntpmake() { make -C "$B" TICKADJ_DB= TICKADJ_MS= CALC_TICKADJ_DB= CALC_TICKADJ_MS= "$@"; }
do_build() { _ntpmake -j6; }
do_install() { _ntpmake DESTDIR="$STAGE" install; }
