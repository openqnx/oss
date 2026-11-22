# perl-html-parser: HTML::Parser, an event-driven HTML parser for Perl (XS), with HTML::Entities and HTML::LinkExtor.
VERSION=3.87
TARBALL=https://cpan.metacpan.org/authors/id/O/OA/OALDERS/HTML-Parser-3.87.tar.gz
SHA256=04342b30b3b1675041b28cfecb693a3d3a0c051587cc40b746a7f9a72a7e3b8a
DEPS="perl"
RUNTIME_DEPS="perl perl-html-tagset perl-http-message perl-uri"
BUILDSYS=custom
source "$WS/scripts/lib/perl-module.sh"
do_configure() { qnx_perl_xs_configure; }
do_build() { qnx_perl_xs_make; }
do_install() { qnx_perl_xs_install; }
