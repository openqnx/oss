# perl-http-daemon: HTTP::Daemon, a simple HTTP/1.1 server class for Perl.
# Pure Perl: lib/ as released into the target perl's site library
# (scripts/lib/perl-module.sh).
VERSION=6.17
TARBALL=https://cpan.metacpan.org/authors/id/O/OA/OALDERS/HTTP-Daemon-6.17.tar.gz
SHA256=16281580c40e23108d028434698b5d7d53637bf904c9df822481e253cbec920c
DEPS="perl"
RUNTIME_DEPS="perl perl-http-date perl-http-message perl-lwp-mediatypes"
BUILDSYS=custom
source "$WS/scripts/lib/perl-module.sh"
do_configure() { :; }
do_build() { :; }
do_install() { qnx_perl_pure_install; }
