# perl-file-sharedir-install: File::ShareDir::Install, installs a Perl distribution's shared data files (a Makefile.PL helper).
# Not in the gap list itself: perl-xml-parser needs it.
# Pure Perl: lib/ as released into the target perl's site library
# (scripts/lib/perl-module.sh).
VERSION=0.14
TARBALL=https://cpan.metacpan.org/authors/id/E/ET/ETHER/File-ShareDir-Install-0.14.tar.gz
SHA256=8f9533b198f2d4a9a5288cbc7d224f7679ad05a7a8573745599789428bc5aea0
DEPS="perl"
RUNTIME_DEPS="perl"
BUILDSYS=custom
source "$WS/scripts/lib/perl-module.sh"
do_configure() { :; }
do_build() { :; }
do_install() { qnx_perl_pure_install; }
