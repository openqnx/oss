VERSION=2.6
TARBALL=https://ftp.gnu.org/gnu/inetutils/inetutils-2.6.tar.xz
SHA256=68bedbfeaf73f7d86be2a7d99bcfbd4093d829f52770893919ae174c0b2357ca
BUILDSYS=autotools
# crypt() for ftpd: QNX keeps it in its static liblogin.a, not libc, so
# libxcrypt (libcrypt.so.2) supplies it; without it configure drops ftpd.
DEPS="ncurses readline libxcrypt"
# The subset of GNU inetutils that builds and works on QNX: telnet and
# telnetd, ftp and ftpd, ping/ping6, traceroute, whois, hostname,
# dnsdomainname, logger. Not built: tftp/tftpd (tftp-hpa is ported and owns
# those names), inetd, syslogd, rsh/rlogin/rexec/rcp and their daemons, talk,
# uucpd, ifconfig (whose interface code is Linux/BSD ioctl-specific).
# telnetd starts QNX's own /bin/login (inetutils would take $bindir/login,
# /usr/local/bin/login, which nothing installs).
# telnetd and ftpd are inetd-style servers (the connection on stdin).
CONF_ARGS_EXTRA=( --disable-servers --disable-clients
                  --enable-telnet --enable-telnetd --enable-ftp --enable-ftpd
                  --enable-ping --enable-ping6 --enable-traceroute --enable-whois
                  --enable-hostname --enable-dnsdomainname --enable-logger
                  --disable-nls --without-pam --without-krb5 --without-shishi
                  --with-path-login=/bin/login )
# ping, ping6 and traceroute are installed setuid root with "install -o
# root -m 4755", which an unprivileged build cannot do: the chown fails and
# leaves the files mode 0600 (not executable). Install them without the
# owner flag; the mode, setuid included, is kept, and scripts/qnxpkg records
# every file as root.
do_install() { make -C "$B" DESTDIR="$STAGE" SUIDMODE="-m 4755" install; }
