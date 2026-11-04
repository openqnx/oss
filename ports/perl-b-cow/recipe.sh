# perl-b-cow: B::COW, Perl's copy-on-write string internals (is a scalar COW, its refcount).
VERSION=0.007
TARBALL=https://cpan.metacpan.org/authors/id/A/AT/ATOOMIC/B-COW-0.007.tar.gz
SHA256=1290daf227e8b09889a31cf182e29106f1cf9f1a4e9bf7752f9de92ed1158b44
DEPS="perl"
RUNTIME_DEPS="perl"
BUILDSYS=custom
source "$WS/scripts/lib/perl-module.sh"
do_configure() { qnx_perl_xs_configure; }
do_build() { qnx_perl_xs_make; }
do_install() { qnx_perl_xs_install; }
