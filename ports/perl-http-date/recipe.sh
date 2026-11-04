# perl-http-date: HTTP::Date, date conversion routines for HTTP (RFC 1123/850, ISO 8601) in Perl.
# Pure Perl: lib/ as released into the target perl's site library
# (scripts/lib/perl-module.sh).
VERSION=6.08
TARBALL=https://cpan.metacpan.org/authors/id/O/OA/OALDERS/HTTP-Date-6.08.tar.gz
SHA256=b57d80ca6d821c6949ca48b27467d45aba7a9c77346562306facca781a003e44
DEPS="perl"
RUNTIME_DEPS="perl"
BUILDSYS=custom
source "$WS/scripts/lib/perl-module.sh"
do_configure() { :; }
do_build() { :; }
do_install() { qnx_perl_pure_install; }
