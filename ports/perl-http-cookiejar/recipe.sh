# perl-http-cookiejar: HTTP::CookieJar, a minimalist RFC 6265 HTTP cookie jar for Perl.
# Pure Perl: lib/ as released into the target perl's site library
# (scripts/lib/perl-module.sh).
VERSION=0.014
TARBALL=https://cpan.metacpan.org/authors/id/D/DA/DAGOLDEN/HTTP-CookieJar-0.014.tar.gz
SHA256=7094ea5c91f536d263b85e83ab4e9a963e11c4408ce08ecae553fa9c0cc47e73
DEPS="perl"
RUNTIME_DEPS="perl perl-http-date"
BUILDSYS=custom
source "$WS/scripts/lib/perl-module.sh"
do_configure() { :; }
do_build() { :; }
do_install() { qnx_perl_pure_install; }
