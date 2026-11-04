# perl-test-requires: Test::Requires, skips Perl tests when required modules are missing.
# Pure Perl: lib/ as released into the target perl's site library
# (scripts/lib/perl-module.sh).
VERSION=0.11
TARBALL=https://cpan.metacpan.org/authors/id/T/TO/TOKUHIROM/Test-Requires-0.11.tar.gz
SHA256=4b88de549597eecddf7c3c38a4d0204a16f59ad804577b671896ac04e24e040f
DEPS="perl"
RUNTIME_DEPS="perl"
BUILDSYS=custom
source "$WS/scripts/lib/perl-module.sh"
do_configure() { :; }
do_build() { :; }
do_install() { qnx_perl_pure_install; }
