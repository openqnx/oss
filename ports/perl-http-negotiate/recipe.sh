# perl-http-negotiate: HTTP::Negotiate, HTTP content negotiation (choose a variant by Accept headers) for Perl.
# Pure Perl: lib/ as released into the target perl's site library
# (scripts/lib/perl-module.sh).
VERSION=6.01
TARBALL=https://cpan.metacpan.org/authors/id/G/GA/GAAS/HTTP-Negotiate-6.01.tar.gz
SHA256=1c729c1ea63100e878405cda7d66f9adfd3ed4f1d6cacaca0ee9152df728e016
DEPS="perl"
RUNTIME_DEPS="perl perl-http-message"
BUILDSYS=custom
source "$WS/scripts/lib/perl-module.sh"
do_configure() { :; }
do_build() { :; }
do_install() { qnx_perl_pure_install; }
