# perl-xml-parser: XML::Parser, the Perl interface to the Expat XML parser (XS).
# Built against this tree's expat (EXPATINCPATH/EXPATLIBPATH name the
# staging tree, so Makefile.PL's pkg-config/multiarch search does not find
# the build machine's). Its Devel::CheckLib probes (is expat there; does it
# have XML_SetAllocTrackerMaximumAmplification) run with the cross
# stand-in (PERL_CROSS_CHECKLIB=1, scripts/lib/perl-module.sh): compiled
# and linked for QNX, never run. Makefile.PL needs File::ShareDir::Install
# on the build machine: the staged perl-file-sharedir-install copy is put on
# its @INC (just that module). The encoding maps (share/*.enc) go to
# auto/share/dist/XML-Parser in the site arch directory, where
# File::ShareDir (run time) finds them.
VERSION=2.59
TARBALL=https://cpan.metacpan.org/authors/id/T/TO/TODDR/XML-Parser-2.59.tar.gz
SHA256=a358fd7c49f5e27717a644a9102bd21dc7fc25a415983279c59b1580e2b62a58
DEPS="perl expat perl-file-sharedir-install"
RUNTIME_DEPS="perl perl-file-sharedir"
BUILDSYS=custom
source "$WS/scripts/lib/perl-module.sh"
do_configure() {
  local h="$B/.hostlib/File/ShareDir"
  mkdir -p "$h" && cp "$STAGE_SYSROOT$(qnx_perl_cfg installsitelib)/File/ShareDir/Install.pm" "$h/" && \
  PERL5LIB="$B/.hostlib" PERL_CROSS_CHECKLIB=1 qnx_perl_xs_configure \
    EXPATINCPATH="$STAGE_SYSROOT$PREFIX/include" EXPATLIBPATH="$STAGE_SYSROOT$PREFIX/lib"
}
do_build() { qnx_perl_xs_make; }
do_install() { qnx_perl_xs_install; }
