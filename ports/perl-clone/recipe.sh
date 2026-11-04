# perl-clone: Clone, recursive deep copy of Perl data structures (XS).
VERSION=0.51
TARBALL=https://cpan.metacpan.org/authors/id/A/AT/ATOOMIC/Clone-0.51.tar.gz
SHA256=f17f66fec97dacca67ac9585701d2d079cfc80539fe6e8160c201c4e55f67507
DEPS="perl"
RUNTIME_DEPS="perl"
BUILDSYS=custom
source "$WS/scripts/lib/perl-module.sh"
do_configure() { qnx_perl_xs_configure; }
do_build() { qnx_perl_xs_make; }
do_install() { qnx_perl_xs_install; }
