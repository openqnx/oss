# perl-test-deep: Test::Deep, deep comparison of data structures with patterns, for Perl tests.
# Pure Perl: lib/ as released into the target perl's site library
# (scripts/lib/perl-module.sh).
VERSION=1.205
TARBALL=https://cpan.metacpan.org/authors/id/R/RJ/RJBS/Test-Deep-1.205.tar.gz
SHA256=42781e9943a7a215e662c4973b9feafdc019fd16469bdb849a8537ee58956273
DEPS="perl"
RUNTIME_DEPS="perl"
BUILDSYS=custom
source "$WS/scripts/lib/perl-module.sh"
do_configure() { :; }
do_build() { :; }
do_install() { qnx_perl_pure_install; }
