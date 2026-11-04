# perl-xml-simple: XML::Simple, reads and writes XML as Perl data structures (on XML::Parser).
# Pure Perl: lib/ as released into the target perl's site library
# (scripts/lib/perl-module.sh).
VERSION=2.25
TARBALL=https://cpan.metacpan.org/authors/id/G/GR/GRANTM/XML-Simple-2.25.tar.gz
SHA256=531fddaebea2416743eb5c4fdfab028f502123d9a220405a4100e68fc480dbf8
DEPS="perl"
RUNTIME_DEPS="perl perl-xml-parser"
BUILDSYS=custom
source "$WS/scripts/lib/perl-module.sh"
do_configure() { :; }
do_build() { :; }
do_install() { qnx_perl_pure_install; }
