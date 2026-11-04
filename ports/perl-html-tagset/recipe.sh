# perl-html-tagset: HTML::Tagset, data tables about HTML elements for Perl HTML parsers.
# Pure Perl: lib/ as released into the target perl's site library
# (scripts/lib/perl-module.sh).
VERSION=3.24
TARBALL=https://cpan.metacpan.org/authors/id/P/PE/PETDANCE/HTML-Tagset-3.24.tar.gz
SHA256=eb89e145a608ed1f8f141a57472ee5f69e67592a432dcd2e8b1dbb445f2b230b
DEPS="perl"
RUNTIME_DEPS="perl"
BUILDSYS=custom
source "$WS/scripts/lib/perl-module.sh"
do_configure() { :; }
do_build() { :; }
do_install() { qnx_perl_pure_install; }
