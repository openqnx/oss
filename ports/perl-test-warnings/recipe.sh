# perl-test-warnings: Test::Warnings, tests for (unexpected) warnings in Perl tests.
# Pure Perl: lib/ as released into the target perl's site library
# (scripts/lib/perl-module.sh).
VERSION=0.038
TARBALL=https://cpan.metacpan.org/authors/id/E/ET/ETHER/Test-Warnings-0.038.tar.gz
SHA256=3da27e003a3c3cafed3c09b42be05cf9bdbff0bee5c8590a731b02853880a273
DEPS="perl"
RUNTIME_DEPS="perl"
BUILDSYS=custom
source "$WS/scripts/lib/perl-module.sh"
do_configure() { :; }
do_build() { :; }
do_install() { qnx_perl_pure_install; }
