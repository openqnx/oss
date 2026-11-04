# perl-extutils-hascompiler: ExtUtils::HasCompiler, checks whether Perl can compile and load an XS module.
# Pure Perl: lib/ as released into the target perl's site library
# (scripts/lib/perl-module.sh).
VERSION=0.025
TARBALL=https://cpan.metacpan.org/authors/id/L/LE/LEONT/ExtUtils-HasCompiler-0.025.tar.gz
SHA256=02e1e7275df00682d3ca8ea1c27a30591d6dd08b3e3865c0958afae911635a66
DEPS="perl"
RUNTIME_DEPS="perl"
BUILDSYS=custom
source "$WS/scripts/lib/perl-module.sh"
do_configure() { :; }
do_build() { :; }
do_install() { qnx_perl_pure_install; }
