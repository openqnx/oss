# perl-io-socket-ssl: IO::Socket::SSL, TLS sockets for Perl on top of Net::SSLeay (OpenSSL).
# Pure Perl: lib/ as released into the target perl's site library
# (scripts/lib/perl-module.sh).
VERSION=2.099
TARBALL=https://cpan.metacpan.org/authors/id/S/SU/SULLR/IO-Socket-SSL-2.099.tar.gz
SHA256=a0be800ff4852b1567ee5500e772417ad7a360abff80c01b5b875c15d44be832
DEPS="perl"
RUNTIME_DEPS="perl perl-net-ssleay perl-mozilla-ca"
BUILDSYS=custom
source "$WS/scripts/lib/perl-module.sh"
do_configure() { :; }
do_build() { :; }
do_install() { qnx_perl_pure_install; }
