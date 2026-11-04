# perl-file-sharedir: File::ShareDir, locates the per-distribution shared data files of Perl modules.
# Not in the gap list itself: perl-xml-parser needs it.
# Pure Perl: lib/ as released into the target perl's site library
# (scripts/lib/perl-module.sh).
VERSION=1.118
TARBALL=https://cpan.metacpan.org/authors/id/R/RE/REHSACK/File-ShareDir-1.118.tar.gz
SHA256=3bb2a20ba35df958dc0a4f2306fc05d903d8b8c4de3c8beefce17739d281c958
DEPS="perl"
RUNTIME_DEPS="perl perl-class-inspector"
BUILDSYS=custom
source "$WS/scripts/lib/perl-module.sh"
do_configure() { :; }
do_build() { :; }
do_install() { qnx_perl_pure_install; }
