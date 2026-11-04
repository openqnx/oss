# icon-naming-utils: icon-name-mapping, the Perl script icon themes run at
# install time to add the legacy (GNOME/KDE) icon names as symlinks to the
# freedesktop.org Icon Naming Specification names, with its mapping data.
# Version as in QNX's own package list.
VERSION=0.8.90
TARBALL=http://tango.freedesktop.org/releases/icon-naming-utils-0.8.90.tar.bz2
SHA256=b1378679df4485b4459f609a3304238b3e92d91e43744c47b70abbe690d883d5
DEPS=""
# The script uses XML::Simple (which parses with XML::Parser) to read the
# mapping.
RUNTIME_DEPS="perl perl-xml-simple perl-xml-parser"
BUILDSYS=autotools
# configure requires XML::Simple in the perl it runs -- the build machine's,
# which may not have it: the perl-xml-simple port's copy in staging (pure
# Perl) is on PERL5LIB for that check.
CONF_ENV=( PERL=/usr/bin/perl PERL5LIB="$STAGE_SYSROOT/usr/local/lib/perl5/site_perl/5.40.2" )
do_install() {
  make -C "$B" DESTDIR="$STAGE" install || return 1
  # #!/usr/bin/perl -w is the build machine's (hazard 105).
  sed -i "1s|^#! */usr/bin/perl|#!$PREFIX/bin/perl|" "$STAGE$PREFIX/libexec/icon-name-mapping"
}
