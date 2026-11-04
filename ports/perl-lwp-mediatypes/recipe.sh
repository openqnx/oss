# perl-lwp-mediatypes: LWP::MediaTypes, guesses the media type (MIME type) of a file or URL for Perl.
# Pure Perl: lib/ as released into the target perl's site library
# (scripts/lib/perl-module.sh).
VERSION=6.05
TARBALL=https://cpan.metacpan.org/authors/id/O/OA/OALDERS/LWP-MediaTypes-6.05.tar.gz
SHA256=abb2dcfbf069317fe65b098e3b2ad58c5eb33e9a839b9190cce6d371f8966cc1
DEPS="perl"
RUNTIME_DEPS="perl"
BUILDSYS=custom
source "$WS/scripts/lib/perl-module.sh"
do_configure() { :; }
do_build() { :; }
do_install() { qnx_perl_pure_install; }
