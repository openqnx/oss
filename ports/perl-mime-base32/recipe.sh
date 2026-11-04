# perl-mime-base32: MIME::Base32, Base32 encoding (RFC 4648) for Perl (URI::otpauth uses it).
# Not in the gap list itself: perl-uri needs it.
# Pure Perl: lib/ as released into the target perl's site library
# (scripts/lib/perl-module.sh).
VERSION=1.303
TARBALL=https://cpan.metacpan.org/authors/id/R/RE/REHSACK/MIME-Base32-1.303.tar.gz
SHA256=ab21fa99130e33a0aff6cdb596f647e5e565d207d634ba2ef06bdbef50424e99
DEPS="perl"
RUNTIME_DEPS="perl"
BUILDSYS=custom
source "$WS/scripts/lib/perl-module.sh"
do_configure() { :; }
do_build() { :; }
do_install() { qnx_perl_pure_install; }
