# perl-test-leaktrace: Test::LeakTrace, finds memory leaks (leaked SVs) in Perl code (XS).
VERSION=0.17
TARBALL=https://cpan.metacpan.org/authors/id/L/LE/LEEJO/Test-LeakTrace-0.17.tar.gz
SHA256=777d64d2938f5ea586300eef97ef03eacb43d4c1853c9c3b1091eb3311467970
DEPS="perl"
RUNTIME_DEPS="perl"
BUILDSYS=custom
source "$WS/scripts/lib/perl-module.sh"
do_configure() { qnx_perl_xs_configure; }
do_build() { qnx_perl_xs_make; }
do_install() { qnx_perl_xs_install; }
