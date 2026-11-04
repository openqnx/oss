# perl-extutils-helpers: ExtUtils::Helpers, portable file and argument helpers for Perl module builders.
# Pure Perl: lib/ as released into the target perl's site library
# (scripts/lib/perl-module.sh).
VERSION=0.028
TARBALL=https://cpan.metacpan.org/authors/id/L/LE/LEONT/ExtUtils-Helpers-0.028.tar.gz
SHA256=c8574875cce073e7dc5345a7b06d502e52044d68894f9160203fcaab379514fe
DEPS="perl"
RUNTIME_DEPS="perl"
BUILDSYS=custom
source "$WS/scripts/lib/perl-module.sh"
do_configure() { :; }
do_build() { :; }
do_install() { qnx_perl_pure_install; }
