# perl-module-build: Module::Build, the Build.PL based build and install system for Perl modules.
# Pure Perl, but Build.PL generates Module::Build::ConfigData (the optional
# features table Module::Build reads on every run): the build machine's perl
# runs Build.PL and ./Build in a copy of the source, and blib/lib -- the
# released modules plus ConfigData.pm -- goes into the target perl's site
# library (scripts/lib/perl-module.sh). The table holds the features'
# descriptions and requirements; whether a feature is available is checked
# at run time on the target.
VERSION=0.4234
TARBALL=https://cpan.metacpan.org/authors/id/L/LE/LEONT/Module-Build-0.4234.tar.gz
SHA256=66aeac6127418be5e471ead3744648c766bd01482825c5b66652675f2bc86a8f
DEPS="perl"
RUNTIME_DEPS="perl"
BUILDSYS=custom
source "$WS/scripts/lib/perl-module.sh"
do_configure() { cp -a "$S/." "$B/" && cd "$B" && PERL_MM_USE_DEFAULT=1 perl Build.PL; }
do_build() { cd "$B" && perl Build; }
do_install() { PERL_LIB_DIR="$B/blib/lib" qnx_perl_pure_install bin/config_data; }
