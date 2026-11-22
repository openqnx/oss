# perl-class-inspector: Class::Inspector, information about loaded Perl classes and their methods.
# Not in the gap list itself: perl-file-sharedir needs it.
# Pure Perl: lib/ as released into the target perl's site library
# (scripts/lib/perl-module.sh).
VERSION=1.37_01
TARBALL=https://cpan.metacpan.org/authors/id/P/PL/PLICEASE/Class-Inspector-1.37_01.tar.gz
SHA256=e426844233416a0f14a6d9ba7ef98851c7ea9971e69a03467b90bb5dd511f278
DEPS="perl"
RUNTIME_DEPS="perl"
BUILDSYS=custom
source "$WS/scripts/lib/perl-module.sh"
do_configure() { :; }
do_build() { :; }
do_install() { qnx_perl_pure_install; }
