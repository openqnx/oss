# perl-html-parser: HTML::Parser, an event-driven HTML parser for Perl (XS), with HTML::Entities and HTML::LinkExtor.
VERSION=3.86
TARBALL=https://cpan.metacpan.org/authors/id/O/OA/OALDERS/HTML-Parser-3.86.tar.gz
SHA256=5305e1d5faa70ee0aa282d85a5c9aa93911e8c7afa590ad1ced5703a544e20f6
DEPS="perl"
RUNTIME_DEPS="perl perl-html-tagset perl-http-message perl-uri"
BUILDSYS=custom
source "$WS/scripts/lib/perl-module.sh"
do_configure() { qnx_perl_xs_configure; }
do_build() { qnx_perl_xs_make; }
do_install() { qnx_perl_xs_install; }
