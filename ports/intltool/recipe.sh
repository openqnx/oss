# intltool: merges translations from gettext .po files into .desktop, XML,
# .schemas and other non-C files (intltool-merge), extracts their strings
# (intltool-extract, intltool-update) and installs the IT_PROG_INTLTOOL
# autoconf macro and po/Makefile.in.in (intltoolize). Perl scripts.
# Version as in QNX's own package list.
VERSION=0.51.0
TARBALL=https://launchpad.net/intltool/trunk/0.51.0/+download/intltool-0.51.0.tar.gz
SHA256=67c74d94196b153b774ab9f89b2fa6c6ba79352407037c8c14d5aeb334e959cd
DEPS=""
# XML::Parser (perl-xml-parser) for the XML merges; intltool-update runs
# gettext's xgettext/msgmerge from PATH (the gettext port), which only
# updating translations needs.
RUNTIME_DEPS="perl perl-xml-parser"
BUILDSYS=autotools
# configure checks that the perl it runs has XML::Parser: the build
# machine's (only that check; the scripts run with the target's perl).
CONF_ENV=( INTLTOOL_PERL=/usr/bin/perl )
do_install() {
  make -C "$B" DESTDIR="$STAGE" install || return 1
  local f
  # The scripts start with the build machine's #!/usr/bin/perl (hazard 105).
  for f in intltool-extract intltool-merge intltool-prepare intltool-update; do
    sed -i "1s|^#! */usr/bin/perl|#!$PREFIX/bin/perl|" "$STAGE$PREFIX/bin/$f" || return 1
  done
}
