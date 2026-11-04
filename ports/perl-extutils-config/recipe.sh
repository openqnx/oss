# perl-extutils-config: ExtUtils::Config, a wrapper for Perl's configuration (used by Perl module builders).
# Pure Perl: lib/ as released into the target perl's site library
# (scripts/lib/perl-module.sh).
VERSION=0.010
TARBALL=https://cpan.metacpan.org/authors/id/L/LE/LEONT/ExtUtils-Config-0.010.tar.gz
SHA256=82e7e4e90cbe380e152f5de6e3e403746982d502dd30197a123652e46610c66d
DEPS="perl"
RUNTIME_DEPS="perl"
BUILDSYS=custom
source "$WS/scripts/lib/perl-module.sh"
do_configure() { :; }
do_build() { :; }
do_install() { qnx_perl_pure_install; }
