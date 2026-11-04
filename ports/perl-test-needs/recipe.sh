# perl-test-needs: Test::Needs, skips Perl tests when optional modules are missing.
# Pure Perl: lib/ as released into the target perl's site library
# (scripts/lib/perl-module.sh).
VERSION=0.002010
TARBALL=https://cpan.metacpan.org/authors/id/H/HA/HAARG/Test-Needs-0.002010.tar.gz
SHA256=923ffdc78fcba96609753e4bae26b0ba0186893de4a63cd5236e012c7c90e208
DEPS="perl"
RUNTIME_DEPS="perl"
BUILDSYS=custom
source "$WS/scripts/lib/perl-module.sh"
do_configure() { :; }
do_build() { :; }
do_install() { qnx_perl_pure_install; }
