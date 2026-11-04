# perl-http-message: HTTP::Message, HTTP request, response and header objects for Perl (libwww-perl).
# Pure Perl: lib/ as released into the target perl's site library
# (scripts/lib/perl-module.sh).
VERSION=7.04
TARBALL=https://cpan.metacpan.org/authors/id/O/OA/OALDERS/HTTP-Message-7.04.tar.gz
SHA256=699f3350dbb7bd8fdc9f3b013b0c91b7c059783708443e39bc395fa33352f006
DEPS="perl"
RUNTIME_DEPS="perl perl-clone perl-encode-locale perl-http-date perl-io-html perl-lwp-mediatypes perl-uri"
BUILDSYS=custom
source "$WS/scripts/lib/perl-module.sh"
do_configure() { :; }
do_build() { :; }
do_install() { qnx_perl_pure_install; }
