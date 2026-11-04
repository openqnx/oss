# perl-class-inspector: Class::Inspector, information about loaded Perl classes and their methods.
# Not in the gap list itself: perl-file-sharedir needs it.
# Pure Perl: lib/ as released into the target perl's site library
# (scripts/lib/perl-module.sh).
VERSION=1.36
TARBALL=https://cpan.metacpan.org/authors/id/P/PL/PLICEASE/Class-Inspector-1.36.tar.gz
SHA256=cc295d23a472687c24489d58226ead23b9fdc2588e522f0b5f0747741700694e
DEPS="perl"
RUNTIME_DEPS="perl"
BUILDSYS=custom
source "$WS/scripts/lib/perl-module.sh"
do_configure() { :; }
do_build() { :; }
do_install() { qnx_perl_pure_install; }
