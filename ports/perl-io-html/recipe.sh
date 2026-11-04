# perl-io-html: IO::HTML, opens an HTML file with its encoding detected (BOM, <meta charset>).
# Pure Perl: lib/ as released into the target perl's site library
# (scripts/lib/perl-module.sh).
VERSION=1.004
TARBALL=https://cpan.metacpan.org/authors/id/C/CJ/CJM/IO-HTML-1.004.tar.gz
SHA256=c87b2df59463bbf2c39596773dfb5c03bde0f7e1051af339f963f58c1cbd8bf5
DEPS="perl"
RUNTIME_DEPS="perl"
BUILDSYS=custom
source "$WS/scripts/lib/perl-module.sh"
do_configure() { :; }
do_build() { :; }
do_install() { qnx_perl_pure_install; }
