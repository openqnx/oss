# perl-test-fatal: Test::Fatal, captures exceptions in Perl tests.
# Pure Perl: lib/ as released into the target perl's site library
# (scripts/lib/perl-module.sh).
VERSION=0.019
TARBALL=https://cpan.metacpan.org/authors/id/R/RJ/RJBS/Test-Fatal-0.019.tar.gz
SHA256=d8f6b6c87bb399a7d60c295a94a4b741a85b23889cfd6b72f1ff62f0c55cf70b
DEPS="perl"
RUNTIME_DEPS="perl perl-try-tiny"
BUILDSYS=custom
source "$WS/scripts/lib/perl-module.sh"
do_configure() { :; }
do_build() { :; }
do_install() { qnx_perl_pure_install; }
