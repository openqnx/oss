# Error.pm (try/catch exception objects for Perl; used by git's Perl
# scripts). Pure Perl, so the module files are installed as released into the
# target perl's library. Version as in the QNX Everywhere 8.0.4r1 image.
VERSION=0.17030
TARBALL=https://cpan.metacpan.org/authors/id/S/SH/SHLOMIF/Error-0.17030.tar.gz
SHA256=34d382276c0fb0d6b38355b94c96a30b12d834d5662eb53f088ee25e3e712924
DEPS="perl"
BUILDSYS=custom
do_build() { :; }
do_install() { install -d "$STAGE$PREFIX/lib/perl5/site_perl/Error" && install -m644 "$S/lib/Error.pm" "$STAGE$PREFIX/lib/perl5/site_perl/" && \
  cp -a "$S/lib/Error/." "$STAGE$PREFIX/lib/perl5/site_perl/Error/"; }
