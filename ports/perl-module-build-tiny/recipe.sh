# perl-module-build-tiny: Module::Build::Tiny, a tiny Build.PL based build tool for Perl modules.
# Pure Perl: lib/ as released into the target perl's site library
# (scripts/lib/perl-module.sh).
VERSION=0.053
TARBALL=https://cpan.metacpan.org/authors/id/L/LE/LEONT/Module-Build-Tiny-0.053.tar.gz
SHA256=3726d622da6f655e88fdf89e4fd597709c44970b47de65082003e8d86b5e193a
DEPS="perl"
RUNTIME_DEPS="perl perl-extutils-config perl-extutils-helpers perl-extutils-installpaths perl-cpan-requirements-dynamic"
BUILDSYS=custom
source "$WS/scripts/lib/perl-module.sh"
do_configure() { :; }
do_build() { :; }
do_install() { qnx_perl_pure_install; }
